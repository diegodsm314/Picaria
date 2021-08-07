#ifndef HOLE_H
#define HOLE_H

#include <QObject>
#include <QPushButton>

class Hole : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(int row READ row WRITE setRow)
    Q_PROPERTY(int col READ col WRITE setCol)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

public:
    enum State {
        EmptyState,
        RedState,
        BlueState,
        SelectableState
    };
    Q_ENUM(State)

    explicit Hole(QWidget *parent = nullptr);
    virtual ~Hole();

    int row() const{ return m_row;}
    int col() const{ return m_col;}

    void setRow(int row){ m_row = row;}
    void setCol(int col){ m_col = col;}

    State state() const { return m_state; }
    void setState(State State);

public slots:
    void reset();

signals:
    void stateChanged(State State);

private:
    State m_state;
    int m_row;
    int m_col;

    static QPixmap stateToPixmap(State state);

private slots:
    void updateHole(State state);

};

#endif // HOLE_H
