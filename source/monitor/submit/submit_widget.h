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
#include "ffmpeg_knobs.h"
#include "time_knobs.h"
#include "maya_knobs.h"
#include "houdini_knobs.h"
#include "misc_knobs.h"

class submit : public QWidget
{
private:
    QWidget *_monitor;
    QVBoxLayout *layout;
    bool savePanel = false;

    combo_box *software_box;
    ffmpeg_knobs *_ffmpeg_knobs;
    time_knobs *_time_knobs;
    maya_knobs *_maya_knobs;
    houdini_knobs *_houdini_knobs;
    misc_knobs *_misc_knobs;

    QPushButton *submit_button;

    void ui();
    void connections();

    void submit_start(QString software);
    void send_job(QJsonArray info);
    void set_software(QString software);
    void panel_save();
    void panel_open();
    void submit_file(QString file);

    void update_ffmpeg_panel();
    void calc_ffmpeg_data(QString file, int *first_frame, int *last_frame,
                          int *task_size);

    void hideEvent(QHideEvent *event) override;

public:
    submit(QWidget *_monitor);
    ~submit();

    void submit_files(QStringList files);
};

#endif // SUBMIT_H
