// Author: Francisco Contreras
// Office: VFX Artist - Senior Compositor
// Website: videovina.com

#ifndef COMBO_BOX_H
#define COMBO_BOX_H

#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QWidget>

class combo_box : public QWidget
{
    Q_OBJECT
private:
    QMenu *menu;
    QLabel *label, *arrow;
    QList<QAction *> actions;
    int index;

    int get_index(QString text) const;
    void update_arrow(QString state);

    void mousePressEvent(QMouseEvent *event) override;

public:
    combo_box();
    ~combo_box();

    void add_item(QString item);
    void add_items(QStringList items);
    void set_current_text(QString text, bool emmit_signal = false);
    void set_index(int index, bool emmit_signal = false);
    void clear();
    void set_disabled(bool disable);

    inline int get_index() const;

    inline void set_label_text(QString label_text);

    inline QString get_current_text() const;
    inline void add_separator();

signals:
    void clicked();
    void text_changed(QString text);
    void index_changed(int index);
};

inline int combo_box::get_index() const
{
    return index;
}

inline void combo_box::set_label_text(QString label_text)
{
    label->setText(label_text);
}

inline QString combo_box::get_current_text() const
{
    return label->text();
}

inline void combo_box::add_separator()
{
    menu->addSeparator();
}

#endif // COMBO_BOX_H
