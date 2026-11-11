#ifndef GROUPSEDIT_H
#define GROUPSEDIT_H

#include <QFrame>
#include <QStringList>

class QLineEdit;
class QCompleter;
class FlowLayout;

//! Newsgroups box: each group is a "chip" (with a cross to remove it), new groups are typed (comma, space,
//! Enter or Tab ends one) or chosen in the suggestions. A name that does not look like a newsgroup is red.
class GroupsEdit : public QFrame
{
    Q_OBJECT

public:
    explicit GroupsEdit(QWidget *parent = nullptr);

    QStringList groups() const;          //!< all the groups, in order (the invalid ones too)
    QStringList invalidGroups() const;
    void setGroups(const QStringList &groups); //!< does not emit changed()
    void retranslate();

    static bool isValidGroup(const QString &group);
    static QStringList suggestions(); //!< the most used groups

signals:
    void changed();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onTextEdited(const QString &text);
    void commitText();

private:
    bool _addGroup(const QString &group);
    void _removeChip(QFrame *chip);
    void _updatePlaceholder();
    void _setFocused(bool focused);
    QFrame *_findChip(const QString &group) const;

    FlowLayout *_flow;
    QLineEdit  *_input;
    QCompleter *_completer;
    QList<QFrame *> _chips;
    bool _loading;
};

#endif // GROUPSEDIT_H
