// Author: Francisco Contreras
// Office: VFX Artist - Senior Compositor
// Website: videovina.com

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "shared_variables.h"
#include "log.h"
#include "options.h"
#include "settings.h"
#include "properties.h"
#include "servers_widget.h"
#include "groups_widget.h"
#include "jobs_widget.h"
#include "tasks_widget.h"
#include "update.h"
#include "general.h"
#include "main_menu.h"
#include "toolbar.h"
#include "submit_widget.h"
#include "layout_widget.h"


using namespace std;

class monitor : public QMainWindow
{
    Q_OBJECT

private:
    shared_variables *shared;

    void setup_ui();

    log_class *log;
    options_class *options;
    settings_class *settings;
    properties_class *properties;

    servers_class *servers;
    groups_class *groups;
    jobs_class *jobs;
    tasks_class *tasks;

    update_class *update;
    general_class *general;
    main_menu_class *main_menu;
    toolbar_class *toolbar;

    layout_widget *_layout_widget;

    submit *_submit;

    void closeEvent(QCloseEvent *event) override;

public:
    monitor(QWidget *parent = 0);
    ~monitor();

    inline QWidget *get_groups_widget() const;
    inline toolbar_class *get_toolbar() const;
    inline properties_class *get_properties() const;
    inline update_class *get_update() const;
    inline jobs_class *get_jobs_widget() const;
};

inline jobs_class *monitor::get_jobs_widget() const
{
    return jobs;
}

inline update_class *monitor::get_update() const
{
    return update;
}

inline properties_class *monitor::get_properties() const
{
    return properties;
}

inline toolbar_class *monitor::get_toolbar() const
{
    return toolbar;
}

inline QWidget *monitor::get_groups_widget() const
{
    return groups;
}

#endif // MAIN_WINDOW_H
