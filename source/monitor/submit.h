#ifndef SUBMIT_H
#define SUBMIT_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>

#include "combo_box.h"
#include "ffmpeg_submit.h"

class submit : public QWidget
{
private:
    QWidget *_monitor;
    QVBoxLayout *layout;
    bool savePanel = false;

    void ui();
    void connections();

    void submit_start(QString software);
    void update_server_groups();
    void set_software(QString software);

    combo_box *software_box;
    ffmpeg_submit *ffmpeg_widget;

    QLineEdit *project_dir_edit;
    QPushButton *project_dir_button;
    QLabel *project_dir_label;

    QPushButton *project_button;
    QLineEdit *project_edit;
    QLabel *project_label;

    QLineEdit *render_node_edit;
    QLabel *render_node_label;

    QLineEdit *job_name;
    combo_box *server_group_box;
    combo_box *priority;
    QLineEdit *task_size_edit;
    QLineEdit *comment_edit;
    QLineEdit *first_frame_edit;
    QLineEdit *last_frame_edit;
    QCheckBox *suspend_box;
    QPushButton *submit_button;

public:
    submit(QWidget *_monitor);
    ~submit();
};

#endif // SUBMIT_H
