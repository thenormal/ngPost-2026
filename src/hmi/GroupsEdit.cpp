#include "GroupsEdit.h"

#include <QCompleter>
#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QStringListModel>
#include <QStyle>
#include <QTimer>
#include <QToolButton>

//! layout that puts the widgets in lines, like words in a text
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent) : QLayout(parent), _spacing(6) {}
    ~FlowLayout() override { qDeleteAll(_items); }

    void addItem(QLayoutItem *item) override { _items.append(item); }
    void insertWidget(int index, QWidget *widget)
    {
        addChildWidget(widget);
        _items.insert(index, new QWidgetItem(widget));
        invalidate();
    }
    int count() const override { return _items.size(); }
    QLayoutItem *itemAt(int index) const override { return _items.value(index); }
    QLayoutItem *takeAt(int index) override
    {
        return index >= 0 && index < _items.size() ? _items.takeAt(index) : nullptr;
    }
    Qt::Orientations expandingDirections() const override { return Qt::Orientations(); }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override { return _doLayout(QRect(0, 0, width, 0), true); }
    void setGeometry(const QRect &rect) override
    {
        QLayout::setGeometry(rect);
        _doLayout(rect, false);
    }
    QSize sizeHint() const override { return minimumSize(); }
    QSize minimumSize() const override
    {
        QSize size;
        for (QLayoutItem *item : _items)
            size = size.expandedTo(item->minimumSize());
        QMargins m = contentsMargins();
        return size + QSize(m.left() + m.right(), m.top() + m.bottom());
    }

private:
    int _doLayout(const QRect &rect, bool testOnly) const
    {
        QMargins m = contentsMargins();
        QRect area = rect.adjusted(m.left(), m.top(), -m.right(), -m.bottom());
        int x = area.x(), y = area.y(), lineHeight = 0;
        for (QLayoutItem *item : _items)
        {
            QSize hint = item->sizeHint();
            bool isInput = item->widget() && item->widget()->objectName() == "groupsInput";
            if (isInput)
                hint.setWidth(qMax(hint.width(), 150));
            if (x + hint.width() > area.right() + 1 && lineHeight > 0)
            {
                x = area.x();
                y += lineHeight + _spacing;
                lineHeight = 0;
            }
            // the input takes what is left of the line
            if (isInput)
                hint.setWidth(qMax(150, area.right() + 1 - x));
            if (!testOnly)
                item->setGeometry(QRect(QPoint(x, y), hint));
            x += hint.width() + _spacing;
            lineHeight = qMax(lineHeight, hint.height());
        }
        return y + lineHeight - rect.y() + m.bottom();
    }

    QList<QLayoutItem *> _items;
    int _spacing;
};


QStringList GroupsEdit::suggestions()
{
    return {
        "alt.binaries.test", "alt.binaries.misc", "alt.binaries.multimedia", "alt.binaries.boneless",
        "alt.binaries.teevee", "alt.binaries.hdtv", "alt.binaries.movies", "alt.binaries.moovee",
        "alt.binaries.music", "alt.binaries.sounds.mp3", "alt.binaries.sounds.flac", "alt.binaries.e-book",
        "alt.binaries.ebook", "alt.binaries.pictures", "alt.binaries.documentaries", "alt.binaries.cd.image",
        "alt.binaries.dvd", "alt.binaries.games", "alt.binaries.warez", "alt.binaries.mom",
        "alt.binaries.ath", "alt.binaries.cores", "alt.binaries.anime", "alt.binaries.comics",
        "alt.binaries.audiobooks", "alt.binaries.tv", "alt.binaries.erotica", "alt.binaries.x264",
        "alt.binaries.multimedia.documentaries", "alt.binaries.linux"
    };
}

bool GroupsEdit::isValidGroup(const QString &group)
{
    // at least two parts separated with dots: letters, digits, + _ -
    static const QRegularExpression re("^[a-z0-9+_-]+(\\.[a-z0-9+_-]+)+$");
    return re.match(group).hasMatch();
}


GroupsEdit::GroupsEdit(QWidget *parent) :
    QFrame(parent), _flow(new FlowLayout(this)), _input(new QLineEdit(this)), _completer(nullptr), _chips(),
    _loading(false)
{
    setObjectName("groupsBox");
    setProperty("focused", false);
    _flow->setContentsMargins(5, 5, 5, 5);

    _input->setObjectName("groupsInput");
    _input->setFrame(false);
    _flow->addWidget(_input);
    setFocusProxy(_input);

    _completer = new QCompleter(suggestions(), this);
    _completer->setCaseSensitivity(Qt::CaseInsensitive);
    _completer->setFilterMode(Qt::MatchContains);
    _input->setCompleter(_completer);
    connect(_completer, QOverload<const QString &>::of(&QCompleter::activated), this, [this](const QString &group) {
        // after the completer has put its text in the input
        QTimer::singleShot(0, this, [this, group]() {
            _input->clear();
            if (_addGroup(group))
                emit changed();
        });
    });

    connect(_input, &QLineEdit::textEdited, this, &GroupsEdit::onTextEdited);
    connect(_input, &QLineEdit::returnPressed, this, &GroupsEdit::commitText);
    _input->installEventFilter(this);
    retranslate();
}

void GroupsEdit::retranslate()
{
    _updatePlaceholder();
    _input->setToolTip(tr("Type a newsgroup and press Enter (or comma, or space). Suggestions appear while you type."));
    for (QFrame *chip : _chips)
        if (chip->property("invalid").toBool())
            chip->setToolTip(tr("This does not look like a newsgroup name (for example alt.binaries.test)"));
}

void GroupsEdit::_updatePlaceholder()
{
    _input->setPlaceholderText(_chips.isEmpty() ? tr("Add a newsgroup, for example alt.binaries.test") : tr("add..."));
}

QStringList GroupsEdit::groups() const
{
    QStringList list;
    for (QFrame *chip : _chips)
        list << chip->property("group").toString();
    return list;
}

QStringList GroupsEdit::invalidGroups() const
{
    QStringList list;
    for (QFrame *chip : _chips)
        if (chip->property("invalid").toBool())
            list << chip->property("group").toString();
    return list;
}

QFrame *GroupsEdit::_findChip(const QString &group) const
{
    for (QFrame *chip : _chips)
        if (chip->property("group").toString() == group)
            return chip;
    return nullptr;
}

bool GroupsEdit::_addGroup(const QString &text)
{
    QString group = text.trimmed().toLower();
    if (group.isEmpty() || _findChip(group))
        return false;

    QFrame *chip = new QFrame(this);
    chip->setObjectName("groupChip");
    chip->setProperty("group", group);
    bool invalid = !isValidGroup(group);
    chip->setProperty("invalid", invalid);
    if (invalid)
        chip->setToolTip(tr("This does not look like a newsgroup name (for example alt.binaries.test)"));

    QHBoxLayout *lay = new QHBoxLayout(chip);
    lay->setContentsMargins(8, 1, 3, 1);
    lay->setSpacing(2);
    QLabel *label = new QLabel(group, chip);
    label->setObjectName("groupChipText");
    QToolButton *close = new QToolButton(chip);
    close->setObjectName("groupChipClose");
    close->setText(QString::fromUtf8("\xc3\x97")); // multiplication sign
    close->setCursor(Qt::PointingHandCursor);
    close->setAutoRaise(true);
    lay->addWidget(label);
    lay->addWidget(close);
    connect(close, &QToolButton::clicked, this, [this, chip]() {
        _removeChip(chip);
        emit changed();
    });

    // before the input, that stays the last one
    _flow->insertWidget(_chips.size(), chip);
    _chips << chip;
    chip->show();
    _updatePlaceholder();
    return true;
}

void GroupsEdit::_removeChip(QFrame *chip)
{
    _chips.removeOne(chip);
    _flow->removeWidget(chip);
    chip->deleteLater();
    _flow->invalidate();
    _updatePlaceholder();
}

void GroupsEdit::setGroups(const QStringList &groups)
{
    for (QFrame *chip : QList<QFrame *>(_chips))
        _removeChip(chip);
    for (const QString &group : groups)
        _addGroup(group);
}

void GroupsEdit::onTextEdited(const QString &text)
{
    static const QRegularExpression separators("[,;\\s]+");
    if (!text.contains(separators))
        return;
    // a separator ends a group: what is typed before is added (also with a paste of a whole list)
    QStringList parts = text.split(separators);
    QString rest = parts.takeLast();
    bool changedSomething = false;
    for (const QString &part : parts)
        changedSomething = _addGroup(part) || changedSomething;
    _input->setText(rest);
    if (changedSomething)
        emit changed();
}

void GroupsEdit::commitText()
{
    QString text = _input->text();
    _input->clear();
    if (_addGroup(text))
        emit changed();
}

void GroupsEdit::_setFocused(bool focused)
{
    setProperty("focused", focused);
    style()->unpolish(this);
    style()->polish(this);
}

bool GroupsEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _input)
    {
        if (event->type() == QEvent::FocusIn)
            _setFocused(true);
        else if (event->type() == QEvent::FocusOut)
        {
            // the popup of the suggestions takes the focus for a moment: not a reason to end the group
            if (static_cast<QFocusEvent *>(event)->reason() != Qt::PopupFocusReason)
            {
                _setFocused(false);
                commitText(); // what is typed is kept when the user goes elsewhere
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *key = static_cast<QKeyEvent *>(event);
            if (key->key() == Qt::Key_Backspace && _input->text().isEmpty() && !_chips.isEmpty())
            {
                _removeChip(_chips.last());
                emit changed();
                return true;
            }
            if (key->key() == Qt::Key_Tab && !_input->text().trimmed().isEmpty())
            {
                commitText();
                return true;
            }
        }
    }
    return QFrame::eventFilter(obj, event);
}

void GroupsEdit::mousePressEvent(QMouseEvent *event)
{
    _input->setFocus();
    QFrame::mousePressEvent(event);
}
