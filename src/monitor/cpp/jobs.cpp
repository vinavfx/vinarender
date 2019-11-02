#include "../hpp/jobs.hpp"

#include <QPixmap>
#include <QtSvg>

jobs_class::jobs_class(
    Ui::MainWindow *_ui,
    shared_variables *_shared,
    QMainWindow *_monitor,
    log_class *_log)
{
    ui = _ui;
    jobs = ui->jobs;
    shared = _shared;
    monitor = _monitor;
    log = _log;

    // Job Acciones
    delete_action = new QAction("Delete");
    job_suspend_action = new QAction("Suspend");
    job_resume_action = new QAction("Resume");
    job_restart_action = new QAction("Restart");
    job_unlock_servers_action = new QAction("Enable blocked servers");
    job_log_action = new QAction("Show Log");
    job_modify_action = new QAction("Modify");
    //------------------------------------------------

    manager_host = fread(path + "/etc/manager_host");

    properties();
    connections();
}

jobs_class::~jobs_class()
{
}

void jobs_class::properties()
{
    jobs->setSelectionMode(QAbstractItemView::ExtendedSelection); // multi seleccion
    jobs->setAlternatingRowColors(true);                          // item con color alternativos
    jobs->setIndentation(0);                                      // elimina el margen del principio

    jobs->setColumnWidth(0, 200); // ajusta el largo de las columnas
    jobs->setColumnWidth(1, 80);
    jobs->setColumnWidth(2, 100);
    jobs->setColumnWidth(3, 200);
    jobs->setColumnWidth(4, 100);
    jobs->setColumnWidth(5, 150);
    jobs->setColumnWidth(6, 150);
    jobs->setColumnWidth(7, 150);
    jobs->setColumnWidth(8, 150);

    jobs->setContextMenuPolicy(Qt::CustomContextMenu);

    jobs->setSortingEnabled(true);
    jobs->sortByColumn(5, Qt::AscendingOrder);
}

void jobs_class::connections()
{
    connect(ui->jobs, &QTreeWidget::itemDoubleClicked, this, &jobs_class::modify);
    connect(ui->jobs, &QTreeWidget::customContextMenuRequested, this, &jobs_class::popup);

    // Job Acciones
    connect(delete_action, &QAction::triggered, this, &jobs_class::item_delete);

    delete_action->setIcon(QIcon(path + "/icons/delete.png"));
    delete_action->setShortcut(QString("Del"));

    connect(job_suspend_action, &QAction::triggered, this, [this]() {
        QString ask = "Sure you want to suspend the job?";
        QString tile = "Job Suspend";
        QString action = "suspend";
        message(&jobs_class::to_action, action, ask, tile, this);
    });

    connect(job_restart_action, &QAction::triggered, this, [this]() {
        QString ask = "Sure you want to restart the job?";
        QString tile = "Job Restart";
        QString action = "restart";
        message(&jobs_class::to_action, action, ask, tile, this);
    });

    job_suspend_action->setIcon(QIcon(path + "/src/monitor/sass/svg/pause.svg"));

    connect(job_resume_action, &QAction::triggered, this, [this]() {
        to_action("resume");
    });
    job_resume_action->setIcon(QIcon(path + "/icons/play3.png"));

    connect(job_unlock_servers_action, &QAction::triggered, this, [this]() {
        to_action("unlock");
    });

    connect(job_log_action, &QAction::triggered, this, &jobs_class::show_log);
    job_log_action->setShortcut(QString("L"));

    connect(job_modify_action, &QAction::triggered, this, &jobs_class::modify);
    job_modify_action->setShortcut(QString("M"));

    connect(ui->opt_ok, &QPushButton::clicked, this, &jobs_class::options_ok);
    connect(ui->opt_cancel, &QPushButton::clicked, [this]() {
        ui->options->hide();
    });

    //-----------------------------------------------------------------------
}

void jobs_class::popup()
{

    auto selected = ui->jobs->selectedItems();
    if (not selected.empty())
    {

        QMenu *menu = new QMenu(monitor);

        menu->addAction(job_suspend_action);
        menu->addAction(job_resume_action);
        menu->addAction(delete_action);
        menu->addAction(job_restart_action);

        menu->addSeparator();
        menu->addAction(job_log_action);
        menu->addAction(job_unlock_servers_action);

        menu->addSeparator();
        menu->addAction(job_modify_action);

        menu->popup(QCursor::pos());
    }
}

void jobs_class::show_log()
{

    auto selected = ui->jobs->selectedItems();
    if (not selected.empty())
    {

        QString job_name = selected[0]->text(0);

        // encuentra job seleccionado en la jobs recividos
        QJsonObject job;
        for (QJsonValue j : shared->jobs)
        {
            QJsonObject _job = j.toObject();
            if (_job["name"].toString() == job_name)
                job = _job;
        }
        //--------------------------------------

        for (QJsonValue t : job["task"].toArray())
        {
            QJsonObject task = t.toObject();
            log_server = task["server"].toString();
            log_server = log_server.split(":")[0];

            if (not(log_server == "..."))
            {

                QStringList vetoed_host;
                QStringList vetoed_name;
                QStringList all_host;
                QStringList all_name;

                for (int i = 0; i < ui->servers->topLevelItemCount(); ++i)
                {
                    auto item = ui->servers->topLevelItem(i);
                    QString name = item->text(0);
                    QString _host = item->text(7);

                    for (QJsonValue vs : job["vetoed_servers"].toArray())
                    {
                        if (name == vs.toString())
                        {
                            vetoed_host.push_back(_host);
                            vetoed_name.push_back(name);
                        }
                    }
                    if (name == log_server)
                    {
                        all_host.push_back(_host);
                        all_name.push_back(name);
                    }
                }

                QString _name;
                QString _host;
                bool failed;
                if (not all_host.empty())
                {
                    _host = all_host[0];
                    _name = all_name[0];
                    failed = false;
                }
                if (not vetoed_host.empty())
                {
                    _host = vetoed_host[0];
                    _name = vetoed_name[0];
                    failed = true;
                }

                QJsonArray send = {_host, QJsonArray({1, failed})};
                QString result = tcpClient(manager_host, 7000, jats({5, send}));

                log->code_editor->setPlainText(_name + " Log:\n\n" + result);
                break;
            }
            else
                log->code_editor->setPlainText("The jobs has not yet rendered");
        }

        ui->log->show();
    }
}

void jobs_class::modify()
{
    ui->options->show();

    // Recive los servers del jobs que estan en el manager
    auto selected = ui->jobs->selectedItems();
    first_job_item = selected[0];

    if (not selected.empty())
    {
        QString job_name = selected.takeLast()->text(0);

        QJsonArray send = {QJsonArray({job_name, "options", "read"})};
        send = {"jobOptions", send};
        QString recv = tcpClient(manager_host, 7000, jats({3, send}));
        QJsonArray pks = jafs(recv);
        //-----------------------------------------

        QStringList serverGroupExist;
        for (QJsonValue sg : pks[1].toArray())
            serverGroupExist.push_back(sg.toString());

        int priority = pks[2].toInt();
        QString comment = pks[3].toString();
        int instances = pks[4].toInt();
        int task_size = pks[5].toInt();
        QString _job_name = pks[6].toString();
        int first_frame = pks[7].toInt();
        int last_frame = pks[8].toInt();

        ui->opt_priority->setCurrentIndex(priority);
        ui->opt_first_frame->setText(QString::number(first_frame));
        ui->opt_last_frame->setText(QString::number(last_frame));
        ui->opt_job_name->setText(_job_name);
        ui->opt_task_size->setText(QString::number(task_size));
        ui->opt_comments->setText(comment);
        ui->opt_instances->setText(QString::number(instances));

        // Agrega grupos al combobox y establese el item correspondiente
        ui->opt_server_group->clear();
        ui->opt_server_group->addItem("None");
        QString current_group = "None";
        for (int i = 0; i < ui->groups->topLevelItemCount(); ++i)
        {
            QString name = ui->groups->topLevelItem(i)->text(2);
            ui->opt_server_group->addItem(name);

            if (serverGroupExist.contains(name))
                current_group = name;
        }
        ui->opt_server_group->setCurrentText(current_group);
        // ---------------------------------
    }
}

void jobs_class::options_ok()
{
    QJsonArray machines = {};
    // guarda el grupo del combobox en el array
    QJsonArray group;
    QString current_group = ui->opt_server_group->currentText();
    group.push_back(current_group);
    // -------------------------------

    int priority = ui->opt_priority->currentIndex();
    int first_frame = ui->opt_first_frame->text().toInt();
    int last_frame = ui->opt_last_frame->text().toInt();
    int task_size = ui->opt_task_size->text().toInt();
    QString comment = ui->opt_comments->text();
    QString _job_name = ui->opt_job_name->text();

    int instances = ui->opt_instances->text().toInt();
    if (instances > 16)
        instances = 16;

    QJsonArray pks;
    auto selected = ui->jobs->selectedItems();
    selected.push_front(first_job_item);

    QStringList repeatItem;
    for (auto item : selected)
    {
        QString job_name = item->text(0);
        QJsonArray options = {machines, group, priority, comment, instances, first_frame, last_frame, task_size, _job_name};
        // el primer item se repite asi que si esta 2 veces no lo agrega otra vez
        if (not repeatItem.contains(job_name))
            pks.push_back({{job_name, options, "write"}});
        repeatItem.push_back(job_name);
        //-------------------------------------------------------
    }
    pks = {"jobOptions", pks};

    QString ask = "Sure you want to send the changes? \nSome frames will be lost due to the size of the tasks.";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(monitor, "Job Options", ask, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        tcpClient(manager_host, 7000, jats({3, pks}));
        ui->options->hide();
    }
}

void jobs_class::message(
    void (jobs_class::*funtion)(QString),
    QString action,
    QString ask,
    QString tile,
    jobs_class *_class)
{
    auto selected = ui->jobs->selectedItems();

    if (not selected.empty())
    {

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(monitor, tile, ask, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            (_class->*funtion)(action);
        }
    }
}

void jobs_class::delete_start(QString action)
{

    auto root = ui->jobs->invisibleRootItem();
    auto selected = ui->jobs->selectedItems();

    QJsonArray pks;

    for (auto item : selected)
    {

        QString job_name = item->text(0);
        os::remove(path + "/log/trayIcon/" + job_name);

        pks.push_back({{job_name, action}});

        root->removeChild(item);
    }

    pks = {"jobAction", pks};

    tcpClient(manager_host, 7000, jats({3, pks}));
}

void jobs_class::to_action(QString action)
{
    shared->stopUpdate = true;

    auto selected = ui->jobs->selectedItems();

    QJsonArray pks;
    for (auto item : selected)
    {
        QString job_name = item->text(0);
        pks.push_back({{job_name, action}});
    }
    pks = {"jobAction", pks};

    tcpClient(manager_host, 7000, jats({3, pks}));
}

void jobs_class::item_delete()
{
    shared->stopUpdate = true;

    QString ask = "Sure you want to delete the job?";
    QString tile = "Job Delete";
    QString action = "delete";
    message(&jobs_class::delete_start, action, ask, tile, this);
}

// void jobs_class::mousePressEvent(QMouseEvent *event)
// {
//     QTreeView::mousePressEvent(event);

//     bool selected = selectionModel()->isSelected(indexAt(event->pos()));
//     // si se clickea no en un item, borra las tareas de la lista
//     if (not selected)
//         ui->tasks->clear();
//     //----------------------------------------------------
// }
