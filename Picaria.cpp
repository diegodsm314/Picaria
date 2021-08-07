#include "Picaria.h"
#include "ui_Picaria.h"

#include <QDebug>
#include <QMessageBox>
#include <QActionGroup>
#include <QSignalMapper>

Picaria::Player state2player(Hole::State state) {
    switch (state) {
        case Hole::RedState:
            return Picaria::RedPlayer;
        case Hole::BlueState:
            return Picaria::BluePlayer;
        default:
            Q_UNREACHABLE();
    }
}

Hole::State player2state(Picaria::Player player) {
    return player == Picaria::RedPlayer ? Hole::RedState : Hole::BlueState;
}

Picaria::Picaria(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Picaria),
      m_mode(Picaria::NineHoles),
      m_player(Picaria::RedPlayer),
      m_phase(Picaria::DropPhase),
      m_dropCount(0),
      m_selected(nullptr){

    ui->setupUi(this);

    QActionGroup* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    modeGroup->addAction(ui->action9holes);
    modeGroup->addAction(ui->action13holes);

    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(reset()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(modeGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateMode(QAction*)));
    QObject::connect(this, SIGNAL(modeChanged(Picaria::Mode)), this, SLOT(reset()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAbout()));

    QSignalMapper* map = new QSignalMapper(this);
    for (int id = 0; id < 13; ++id) {
        int r;
        int c;
        switch(id){
        case 0: r=0; c=0; break;
        case 1: r=0; c=1; break;
        case 2: r=0; c=2; break;
        case 3: r=1; c=0; break;
        case 4: r=1; c=1; break;
        case 5: r=2; c=0; break;
        case 6: r=2; c=1; break;
        case 7: r=2; c=2; break;
        case 8: r=3; c=0; break;
        case 9: r=3; c=1; break;
        case 10: r=4; c=0; break;
        case 11: r=4; c=1; break;
        case 12: r=4; c=2; break;
        }
        QString holeName = QString("hole%1").arg(id+1, 2, 10, QChar('0'));
        Hole* hole = this->findChild<Hole*>(holeName);
        Q_ASSERT(hole != nullptr);
        hole->setRow(r);
        hole->setCol(c);
        m_holes[id] = hole;
        map->setMapping(hole, id);
        QObject::connect(hole, SIGNAL(clicked(bool)), map, SLOT(map()));
    }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
#else
    QObject::connect(map, SIGNAL(mappedInt(int)), this, SLOT(play(int)));
#endif

    this->reset();

    this->adjustSize();
    this->setFixedSize(this->size());
}

Picaria::~Picaria() {
    delete ui;
}

void Picaria::setMode(Picaria::Mode mode) {
    if (m_mode != mode) {
        m_mode = mode;
        emit modeChanged(mode);
    }
}


void Picaria::switchPlayer() {
    m_player = m_player == Picaria::RedPlayer ?
                    Picaria::BluePlayer : Picaria::RedPlayer;
    this->updateStatusBar();
}

void Picaria::play(int id) {
    Hole* hole = m_holes[id];

    qDebug() << "clicked on: " << hole->objectName();
    switch(m_phase){
    case Picaria::DropPhase:
        stateOne(hole);
        break;
    case Picaria::MovePhase:
        stateTwo(hole);
    }
    if(isGameOver(Picaria::RedPlayer) || isGameOver(Picaria::BluePlayer)){
        this->switchPlayer();
        gameOver(m_player);
     }
}

void Picaria::stateOne(Hole* hole){
    if(hole->state()==Hole::EmptyState){
        hole->setState(player2state(m_player));
        this->switchPlayer();
        m_dropCount++;
    }
    if(m_dropCount==6){
        m_phase=MovePhase;
        this->updateStatusBar();
    }

}



void Picaria::reset() {
    // Reset each hole.
    for (int id = 0; id < 13; ++id) {
        Hole* hole = m_holes[id];
        hole->reset();

        // Set the hole visibility according to the board mode.
        switch (id) {
            case 3:
            case 4:
            case 8:
            case 9:
                hole->setVisible(m_mode == Picaria::ThirteenHoles);
                break;
            default:
                break;
        }
    }

    // Reset the player and phase.
    m_player = Picaria::RedPlayer;
    m_phase = Picaria::DropPhase;
    m_dropCount=0;

    // Finally, update the status bar.
    this->updateStatusBar();
}

void Picaria::showAbout() {
    QMessageBox::information(this, tr("About"), tr("Picaria\n\nDiego Simões - diegosean93@gmail.com\nRosane Silva - rosanesfaraujo@gmail.com"));
}

void Picaria::updateMode(QAction* action) {
    if (action == ui->action9holes)
        this->setMode(Picaria::NineHoles);
    else if (action == ui->action13holes)
        this->setMode(Picaria::ThirteenHoles);
    else
        Q_UNREACHABLE();
}

void Picaria::updateStatusBar() {
    QString player(m_player == Picaria::RedPlayer ? "vermelho" : "azul");
    QString phase(m_phase == Picaria::DropPhase ? "colocar" : "mover");

    ui->statusbar->showMessage(tr("Fase de %1: vez do jogador %2").arg(phase).arg(player));
}

bool isSelectable(Hole* hole){
    return hole!=nullptr && (hole->state()==Hole::EmptyState || hole->state()==Hole::SelectableState);
}

QList<Hole*> Picaria::findSelectable(Hole* hole){
    QList<Hole*> list;
    int id;
    switch(hole->row()){
    case 0: id = hole->col();break;
    case 1: id = 3 + hole->col();break;
    case 2: id = 5 + hole->col();break;
    case 3: id = 8 + hole->col();break;
    case 4: id = 10 + hole->col();break;
    }

    //qDebug() << "Row"<< hole->row() << "col"<< hole->col() << "id" <<id;

    if(m_mode == Picaria::NineHoles){
        switch (id)
        {
        case 0:

            if (isSelectable(holeAt(1)))
            { //1
                holeAt(1)->setState(Hole::SelectableState);
                list << holeAt(1);
            }

            if (isSelectable(holeAt(5)))
            { //5
                holeAt(5)-> setState(Hole::SelectableState);
                list << holeAt(5);
            }
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            break;
        case 1:
            if (isSelectable(holeAt(0)))
            { //0
                holeAt(0)->setState(Hole::SelectableState);
                list << holeAt(0);
            }
            if (isSelectable(holeAt(2)))
            { //2
                holeAt(2)->setState(Hole::SelectableState);
                list << holeAt(2);
            }
            if (isSelectable(holeAt(5)))
            { //5
                holeAt(5)->setState(Hole::SelectableState);
                list << holeAt(5);
            }
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(7)))
            { //7
                holeAt(7)->setState(Hole::SelectableState);
                list << holeAt(7);
            }
            break;
        case 2:
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(1)))
            { //1
                holeAt(1)->setState(Hole::SelectableState);
                list << holeAt(1);
            }
            if (isSelectable(holeAt(7)))
            { //7

          holeAt(7)-> setState(Hole::SelectableState);
          list << holeAt(7);
            }
            break;
        case 5:
            if (isSelectable(holeAt(0)))
            { //0
                holeAt(0)->setState(Hole::SelectableState);
                list << holeAt(0);
            }
            if (isSelectable(holeAt(1)))
            { //1
                holeAt(1)->setState(Hole::SelectableState);
                list << holeAt(1);
            }
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(10)))
            { //10
                holeAt(10)->setState(Hole::SelectableState);
                list << holeAt(10);
            }
            if (isSelectable(holeAt(11)))
            { //11
                holeAt(11)->setState(Hole::SelectableState);
                list << holeAt(11);
            }
            break;
        case 6:
            if (isSelectable(holeAt(0)))
            { //0
                holeAt(0)->setState(Hole::SelectableState);
                list << holeAt(0);
            }
            if (isSelectable(holeAt(1)))
            { //1
                holeAt(1)->setState(Hole::SelectableState);
                list << holeAt(1);
            }
            if (isSelectable(holeAt(2)))
            { //2
                holeAt(2)->setState(Hole::SelectableState);
                list << holeAt(2);
            }
            if (isSelectable(holeAt(5)))
            { //5
                holeAt(5)->setState(Hole::SelectableState);
                list << holeAt(5);
            }
            if (isSelectable(holeAt(7)))
            { //7
                holeAt(7)->setState(Hole::SelectableState);
                list << holeAt(7);
            }
            if (isSelectable(holeAt(10)))
            { //10
                holeAt(10)->setState(Hole::SelectableState);
                list << holeAt(10);
            }
            if (isSelectable(holeAt(11)))
            { //11
                holeAt(11)->setState(Hole::SelectableState);
                list << holeAt(11);
            }
            if (isSelectable(holeAt(12)))
            { //12
                holeAt(12)->setState(Hole::SelectableState);
                list << holeAt(12);
            }
            break;
        case 7:
            if (isSelectable(holeAt(1)))
            { //1
                holeAt(1)->setState(Hole::SelectableState);
                list << holeAt(1);
            }
            if (isSelectable(holeAt(2)))
            { //2
                        holeAt(2)-> setState(Hole::SelectableState);
                        list << holeAt(2);
            }
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(11)))
            { //11
                holeAt(11)->setState(Hole::SelectableState);
                list << holeAt(11);
            }
            if (isSelectable(holeAt(12)))
            { //12
                        holeAt(12)-> setState(Hole::SelectableState);
                        list << holeAt(12);
            }
            break;
        case 10:
            if (isSelectable(holeAt(5)))
            { //5
                holeAt(5)->setState(Hole::SelectableState);
                list << holeAt(5);
            }
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(11)))
            { //11
                holeAt(11)->setState(Hole::SelectableState);
                list << holeAt(11);
            }
            break;
        case 11:
            if (isSelectable(holeAt(5)))
            { //5
                holeAt(5)->setState(Hole::SelectableState);
                list << holeAt(5);
            }
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(7)))
            { //7
                holeAt(7)->setState(Hole::SelectableState);
                list << holeAt(7);
            }
            if (isSelectable(holeAt(10)))
            { //10
                holeAt(10)->setState(Hole::SelectableState);
                list << holeAt(10);
            }
            if (isSelectable(holeAt(12)))
            { //12
                holeAt(12)->setState(Hole::SelectableState);
                list << holeAt(12);
            }
            break;
        case 12:
            if (isSelectable(holeAt(6)))
            { //6
                holeAt(6)->setState(Hole::SelectableState);
                list << holeAt(6);
            }
            if (isSelectable(holeAt(7)))
            { //7
                holeAt(7)->setState(Hole::SelectableState);
                list << holeAt(7);
            }
            if (isSelectable(holeAt(11)))
            { //11
                holeAt(11)->setState(Hole::SelectableState);
                list << holeAt(11);
            }
            break;

        }

    }
    else{
        switch (id)
               {
               case 0:
                   if (isSelectable(holeAt(1)))
                   { //1
                       holeAt(1)->setState(Hole::SelectableState);
                       list << holeAt(1);
                   }

                   if (isSelectable(holeAt(5)))
                   { //5
                       holeAt(5)-> setState(Hole::SelectableState);
                       list << holeAt(5);
                   }
                   if (isSelectable(holeAt(3)))
                   { //3
                       holeAt(3)->setState(Hole::SelectableState);
                       list << holeAt(3);
                   }
                   break;
               case 1:
                   if (isSelectable(holeAt(0)))
                   { //0
                       holeAt(0)->setState(Hole::SelectableState);
                       list << holeAt(0);
                   }
                   if (isSelectable(holeAt(2)))
                   { //2
                       holeAt(2)->setState(Hole::SelectableState);
                       list << holeAt(2);
                   }
                   if (isSelectable(holeAt(3)))
                   { //3
                       holeAt(3)->setState(Hole::SelectableState);
                       list << holeAt(3);
                   }
                   if (isSelectable(holeAt(6)))
                   { //6
                       holeAt(6)->setState(Hole::SelectableState);
                       list << holeAt(6);
                   }
                   if (isSelectable(holeAt(4)))
                   { //4
                       holeAt(4)->setState(Hole::SelectableState);
                       list << holeAt(4);
                   }
                   break;
               case 2:
                   if (isSelectable(holeAt(4)))
                   { //4
                       holeAt(4)->setState(Hole::SelectableState);
                       list << holeAt(4);
                   }
                   if (isSelectable(holeAt(1)))
                   { //1
                       holeAt(1)->setState(Hole::SelectableState);
                       list << holeAt(1);
                   }
                   if (isSelectable(holeAt(7)))
                   { //7

                 holeAt(7)-> setState(Hole::SelectableState);
                 list << holeAt(7);
                   }
                   break;
               case 3:
                   if (isSelectable(holeAt(0)))
                   { //0
                       holeAt(0)->setState(Hole::SelectableState);
                       list << holeAt(0);
                   }
                   if (isSelectable(holeAt(1)))
                   { //1
                       holeAt(1)->setState(Hole::SelectableState);
                       list << holeAt(1);
                   }
                   if (isSelectable(holeAt(5)))
                   { //5
                       holeAt(5)-> setState(Hole::SelectableState);
                       list << holeAt(5);
                   }
                   if (isSelectable(holeAt(6)))
                   { //6
                       holeAt(6)-> setState(Hole::SelectableState);
                       list << holeAt(6);
                   }
                   break;
               case 4:
                   if (isSelectable(holeAt(2)))
                   { //2
                       holeAt(2)->setState(Hole::SelectableState);
                       list << holeAt(2);
                   }
                   if (isSelectable(holeAt(1)))
                   { //1
                       holeAt(1)->setState(Hole::SelectableState);
                       list << holeAt(1);
                   }
                   if (isSelectable(holeAt(7)))
                   { //7
                       holeAt(7)-> setState(Hole::SelectableState);
                       list << holeAt(7);
                   }
                   if (isSelectable(holeAt(6)))
                   { //6
                       holeAt(6)-> setState(Hole::SelectableState);
                       list << holeAt(6);
                   }
                   break;
               case 5:
                   if (isSelectable(holeAt(0)))
                   { //0
                       holeAt(0)->setState(Hole::SelectableState);
                       list << holeAt(0);
                   }
                   if (isSelectable(holeAt(3)))
                   { //3
                       holeAt(3)->setState(Hole::SelectableState);
                       list << holeAt(3);
                   }
                   if (isSelectable(holeAt(6)))
                   { //6
                       holeAt(6)->setState(Hole::SelectableState);
                       list << holeAt(6);
                   }
                   if (isSelectable(holeAt(10)))
                   { //10
                       holeAt(10)->setState(Hole::SelectableState);
                       list << holeAt(10);
                   }
                   if (isSelectable(holeAt(8)))
                   { //8
                       holeAt(8)->setState(Hole::SelectableState);
                       list << holeAt(8);
                   }
                   break;
               case 6:
                   if (isSelectable(holeAt(1)))
                   { //1
                       holeAt(1)->setState(Hole::SelectableState);
                       list << holeAt(1);
                   }
                   if (isSelectable(holeAt(4)))
                   { //4
                       holeAt(4)->setState(Hole::SelectableState);
                       list << holeAt(4);
                   }
                   if (isSelectable(holeAt(7)))
                   { //7
                       holeAt(7)->setState(Hole::SelectableState);
                       list << holeAt(7);
                   }
                   if (isSelectable(holeAt(9)))
                   { //9
                       holeAt(9)->setState(Hole::SelectableState);
                       list << holeAt(9);
                   }
                   if (isSelectable(holeAt(11)))
                   { //11
                       holeAt(11)->setState(Hole::SelectableState);
                       list << holeAt(11);
                   }
                   if (isSelectable(holeAt(8)))
                   { //8
                       holeAt(8)->setState(Hole::SelectableState);
                       list << holeAt(8);
                   }
                   if (isSelectable(holeAt(5)))
                   { //5
                       holeAt(5)->setState(Hole::SelectableState);
                       list << holeAt(5);
                   }
                   if (isSelectable(holeAt(3)))
                   { //3
                       holeAt(3)->setState(Hole::SelectableState);
                       list << holeAt(3);
                   }
                   break;
               case 7:
                   if (isSelectable(holeAt(4)))
                   { //4
                       holeAt(4)->setState(Hole::SelectableState);
                       list << holeAt(4);
                   }
                   if (isSelectable(holeAt(2)))
                   { //2
                               holeAt(2)-> setState(Hole::SelectableState);
                               list << holeAt(2);
                   }
                   if (isSelectable(holeAt(6)))
                   { //6
                       holeAt(6)->setState(Hole::SelectableState);
                       list << holeAt(6);
                   }
                   if (isSelectable(holeAt(9)))
                   { //9
                       holeAt(9)->setState(Hole::SelectableState);
                       list << holeAt(9);
                   }
                   if (isSelectable(holeAt(12)))
                   { //12
                               holeAt(12)-> setState(Hole::SelectableState);
                               list << holeAt(12);
                   }
                   break;
            case 8:
                if (isSelectable(holeAt(5)))
                { //5
                    holeAt(5)->setState(Hole::SelectableState);
                    list << holeAt(5);
                }
                if (isSelectable(holeAt(6)))
                { //6
                    holeAt(6)->setState(Hole::SelectableState);
                    list << holeAt(6);
                }
                if (isSelectable(holeAt(10)))
                { //10
                    holeAt(10)-> setState(Hole::SelectableState);
                    list << holeAt(10);
                }
                if (isSelectable(holeAt(11)))
                { //11
                    holeAt(11)-> setState(Hole::SelectableState);
                    list << holeAt(11);
                }
                break;
            case 9:
                if (isSelectable(holeAt(6)))
                { //6
                    holeAt(6)->setState(Hole::SelectableState);
                    list << holeAt(6);
                }
                if (isSelectable(holeAt(11)))
                { //11
                    holeAt(11)->setState(Hole::SelectableState);
                    list << holeAt(11);
                }
                if (isSelectable(holeAt(7)))
                { //7
                    holeAt(7)-> setState(Hole::SelectableState);
                    list << holeAt(7);
                }
                if (isSelectable(holeAt(12)))
                { //12
                    holeAt(12)-> setState(Hole::SelectableState);
                    list << holeAt(12);
                }
                break;
               case 10:
                   if (isSelectable(holeAt(5)))
                   { //5
                       holeAt(5)->setState(Hole::SelectableState);
                       list << holeAt(5);
                   }
                   if (isSelectable(holeAt(8)))
                   { //8
                       holeAt(8)->setState(Hole::SelectableState);
                       list << holeAt(8);
                   }
                   if (isSelectable(holeAt(11)))
                   { //11
                       holeAt(11)->setState(Hole::SelectableState);
                       list << holeAt(11);
                   }
                   break;
               case 11:
                   if (isSelectable(holeAt(8)))
                   { //8
                       holeAt(8)->setState(Hole::SelectableState);
                       list << holeAt(8);
                   }
                   if (isSelectable(holeAt(6)))
                   { //6
                       holeAt(6)->setState(Hole::SelectableState);
                       list << holeAt(6);
                   }
                   if (isSelectable(holeAt(9)))
                   { //9
                       holeAt(9)->setState(Hole::SelectableState);
                       list << holeAt(9);
                   }
                   if (isSelectable(holeAt(10)))
                   { //10
                       holeAt(10)->setState(Hole::SelectableState);
                       list << holeAt(10);
                   }
                   if (isSelectable(holeAt(12)))
                   { //12
                       holeAt(12)->setState(Hole::SelectableState);
                       list << holeAt(12);
                   }
                   break;
               case 12:
                   if (isSelectable(holeAt(9)))
                   { //9
                       holeAt(9)->setState(Hole::SelectableState);
                       list << holeAt(9);
                   }
                   if (isSelectable(holeAt(7)))
                   { //7
                       holeAt(7)->setState(Hole::SelectableState);
                       list << holeAt(7);
                   }
                   if (isSelectable(holeAt(11)))
                   { //11
                       holeAt(11)->setState(Hole::SelectableState);
                       list << holeAt(11);
                   }
                   break;

               }
    }
    return list;
}



Hole* Picaria::holeAt(int index){
        return m_holes[index];
}

void Picaria::stateTwo(Hole *hole){
    qDebug() << m_player;

        QList<Hole*>  selectable;
        if(hole->state() == player2state(m_player)){
            jogar = true;
            selectable = this->findSelectable(hole);
            qDebug() << selectable;
            m_selected = hole;
        }
        else if(jogar){
            jogar = false;
            if(hole->state()==Hole::SelectableState){
                hole->setState(player2state(m_player));
                hole=m_selected;
                hole->setState(Hole::EmptyState);
                this->clearSelectable();
                this->switchPlayer();
            }
            else{
                jogar = false;
                QString player(m_player == Picaria::RedPlayer ? "vermelho" : "azul");
                ui->statusbar->showMessage(tr("Buraco incorreto. Escolha a peça e tente novamente jogador %1").arg(player));
                this->clearSelectable();
            }
        }

}

void Picaria::clearSelectable(){
    for (int id=0; id<13; id++) {
        Hole* hole = m_holes[id];
        if(hole->state()==Hole::SelectableState){
            hole->setState(Hole::EmptyState);
        }
    }
}

bool Picaria::isGameOver(Player player){
    return this->checkRow(player) || this->checkCol(player) ||
               this->checkDiagonal(player) || this->checkAntiDiagonal(player);
}

bool Picaria::checkCol(Player player){
    Hole::State state = player2state(player);
    for (int i=0;i<3 ;i++ ) {
        if(holeAt(0+i)->state()==state &&
                holeAt(5+i)->state()==state &&
                holeAt(10+i)->state()==state){
            return true;
        }
    }
    return false;
}

bool Picaria::checkRow(Player player){
    Hole::State state = player2state(player);
    for (int i=0;i<13 ;i+=5 ) {
        if(holeAt(0+i)->state()==state &&
                holeAt(1+i)->state()==state &&
                holeAt(2+i)->state()==state){
            return true;
        }
    }
    return false;
}

bool Picaria::checkDiagonal(Player player){
    Hole::State state = player2state(player);
    if(m_mode==Picaria::NineHoles){
        if(holeAt(0)->state()==state &&
                holeAt(6)->state()==state &&
                holeAt(12)->state()==state)
            return true;
    }
    else{
        for (int i=0;i<13 ;i+=3) {
            if(holeAt(0+i)->state()==state &&
                    holeAt(3+i)->state()==state &&
                    holeAt(6+i)->state()==state)
                return true;
        }
        if(holeAt(5)->state()==state &&
                holeAt(8)->state()==state &&
                holeAt(11)->state()==state)
            return true;
        else{
            if(holeAt(1)->state()==state &&
                holeAt(4)->state()==state &&
                holeAt(7)->state()==state)
            return true;
        }
    }
    return false;
}

bool Picaria::checkAntiDiagonal(Player player){
    Hole::State state = player2state(player);
    if(m_mode==Picaria::NineHoles){
        if(holeAt(2)->state()==state &&
                holeAt(6)->state()==state &&
                holeAt(10)->state()==state)
            return true;
    }
    else{
        for (int i=2;i<9 ;i+=2) {
            if(holeAt(0+i)->state()==state &&
                    holeAt(2+i)->state()==state &&
                    holeAt(4+i)->state()==state)
                return true;
        }
        if(holeAt(1)->state()==state &&
                holeAt(3)->state()==state &&
                holeAt(5)->state()==state)
            return true;
        else{
            if(holeAt(7)->state()==state &&
                holeAt(9)->state()==state &&
                holeAt(11)->state()==state)
            return true;
        }
    }
    return false;
}

void Picaria::gameOver(Player player){
    switch(player){
    case Picaria::RedPlayer:
        QMessageBox::information(this,tr("Vencedor!"),tr("Parabens jogador vermelho!!!\n Fim de jogo."));   break;
    case Picaria::BluePlayer:
        QMessageBox::information(this,tr("Vencedor!"),tr("Parabens jogador azul!!!\n Fim de jogo."));
    }
    this->reset();
}

