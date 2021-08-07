#ifndef PICARIA_H
#define PICARIA_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class Picaria;
}
QT_END_NAMESPACE

class Hole;

class Picaria : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(Picaria::Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum Mode {
        NineHoles,
        ThirteenHoles
    };
    Q_ENUM(Mode)

    enum Player {
        RedPlayer,
        BluePlayer
    };
    Q_ENUM(Player)

    enum Phase {
        DropPhase,
        MovePhase
    };
    Q_ENUM(Phase)

    Picaria(QWidget *parent = nullptr);
    virtual ~Picaria();

    bool jogar = false;

    Picaria::Mode mode() const { return m_mode; }
    void setMode(Picaria::Mode mode);

    Hole* holeAt(int index);
    QList<Hole*> findSelectable(Hole* hole);

    //void move(Hole* hole);
    void clearSelectable();

    bool isGameOver(Player player);

    bool checkRow(Player player);
    bool checkCol(Player player);
    bool checkAntiDiagonal(Player player);
    bool checkDiagonal(Player player);
    void gameOver(Player player);
    void stateOne(Hole* hole);
    void stateTwo(Hole* hole);


signals:
    void modeChanged(Picaria::Mode mode);

private:
    Ui::Picaria *ui;
    Hole* m_holes[13];
    Mode m_mode;
    Player m_player;
    Phase m_phase;
    int m_dropCount;
    Hole* m_selected;

    void switchPlayer();

private slots:
    void play(int id);
    void reset();

    void showAbout();

    void updateMode(QAction* action);
    void updateStatusBar();

};

#endif // PICARIA_H
