// Author: Francisco Contreras
// Office: VFX Artist - Senior Compositor
// Website: videovina.com

#ifndef OPTIONS_H
#define OPTIONS_H

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "time_knobs.h"
#include "misc_knobs.h"
#include "ffmpeg_knobs.h"
#include "maya_knobs.h"
#include "houdini_knobs.h"
#include "shared_variables.h"

class options_class : public QWidget
{
private:
    QWidget *_monitor;
    shared_variables *shared;

    time_knobs *_time_knobs;
    misc_knobs *_misc_knobs;
    ffmpeg_knobs *_ffmpeg_knobs;
    maya_knobs *_maya_knobs;
    houdini_knobs * _houdini_knobs;

    QStringList selected_jobs;

    QLabel *selected_jobs_label;
    QWidget *box_dialog;

    void setup_ui();
    void save();
    void uptate_panel_from_job(QString job_name);
    void set_disabled_all(bool disable);

public:
    options_class(QWidget *monitor, shared_variables *shared);
    ~options_class();

    void open_panel();
    void update_panel();
};

#endif // OPTIONS_H
