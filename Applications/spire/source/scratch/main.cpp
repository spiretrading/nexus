#include <QApplication>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  

  auto w = new QWidget();
  //w->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  w->setBaseSize(640, 480);

  //QColor hsvColor[6];
  //hsvColor[0].setHsv(0, 255, 255);
  //hsvColor[1].setHsv(60, 255, 255);
  //hsvColor[2].setHsv(120, 255, 255);
  //hsvColor[3].setHsv(180, 255, 255);
  //hsvColor[4].setHsv(240, 255, 255);
  //hsvColor[5].setHsv(300, 255, 255);
  //auto styleH = QString("background-color: qradialgradient(spread:radial, x1:0, y1:0, x2:0.5, y2:0,") +
  //               QString("stop:0.000 rgba(%1, %2, %3, 255),").arg( hsvColor[0].red() ).arg( hsvColor[0].green() ).arg( hsvColor[0].blue() ) +
  //               QString("stop:0.166 rgba(%1, %2, %3, 255),").arg( hsvColor[1].red() ).arg( hsvColor[1].green() ).arg( hsvColor[1].blue() ) +
  //               QString("stop:0.333 rgba(%1, %2, %3, 255),").arg( hsvColor[2].red() ).arg( hsvColor[2].green() ).arg( hsvColor[2].blue() ) +
  //               QString("stop:0.500 rgba(%1, %2, %3, 255),").arg( hsvColor[3].red() ).arg( hsvColor[3].green() ).arg( hsvColor[3].blue() ) +
  //               QString("stop:0.666 rgba(%1, %2, %3, 255),").arg( hsvColor[4].red() ).arg( hsvColor[4].green() ).arg( hsvColor[4].blue() ) +
  //               QString("stop:0.833 rgba(%1, %2, %3, 255),").arg( hsvColor[5].red() ).arg( hsvColor[5].green() ).arg( hsvColor[5].blue() ) +
  //               QString("stop:1.000 rgba(%1, %2, %3, 255));").arg( hsvColor[0].red() ).arg( hsvColor[0].green() ).arg( hsvColor[0].blue() );
  //w->setStyleSheet(styleH);

  //
  auto l = new QVBoxLayout(w);
  l->setMargin(50);
  
  auto *effect = new QGraphicsDropShadowEffect();
  effect->setBlurRadius(50);
  effect->setXOffset(0);
  effect->setYOffset(0);
  effect->setColor(QColor(0, 0, 0, 70));
  auto w2 = new QWidget();
  w2->setGraphicsEffect(effect);
  w2->setStyleSheet("background-color: #F5F5F5; border: 1px solid #8C8C8C;");
  w->setStyleSheet("background-color: #FFFFFF;");
  l->addWidget(w2);

  //auto w2 = new QWidget();
  //w2->setStyleSheet(R"(
  //  background-color: black;
  //  border: 15px solid rgb(255, 255, 255, 180);)");
  //l->addWidget(w2);

  //auto l2 = new QHBoxLayout(w2);
  //
  //auto w3 = new QWidget();
  //w3->setStyleSheet("background-color: yellow;");
  //l2->addWidget(w3);

  w->show();

  application->exec();
}
