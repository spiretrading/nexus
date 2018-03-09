#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>
#include "spire/spire/resources.hpp"

using namespace spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  

  auto w = new QFrame();
  //w->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  w->setBaseSize(640, 480);
  w->setStyleSheet(R"(

    padding: 1px;
    margin: 0px;
    spacing: 0px;
    border-radius:20px;
    border: 20px solid #0080ff;
    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, x3:3, y3:0stop:0 rgba(0, 128, 255, 32), stop:1 rgba(0, 0, 0, 0), stop:2 rgba(255, 0, 0, 25) )
  )");
  w->show();

  auto w2 = new QWidget();
  auto l = new QHBoxLayout(w);
  l->addWidget(w2);

  //QWidget *win = new QWidget();
  //QLabel *label = new QLabel("Box");
  //QLabel *label1 = new QLabel("Panel");
  //QLabel *label2 = new QLabel("Winpanel");
  //QLabel *label3 = new QLabel("H line");
  //QLabel *label4 = new QLabel("V line");
  //QLabel *label5 = new QLabel("Styled Panel");

  //label->setFrameStyle(QFrame::Box | QFrame::Raised);
  //label->setLineWidth(2); label1->setFrameStyle(QFrame::Panel | QFrame::Raised);
  //label1->setLineWidth(2); label2->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  //label2->setLineWidth(2); label3->setFrameStyle(QFrame::HLine | QFrame::Raised);
  //label3->setLineWidth(2); label4->setFrameStyle(QFrame::VLine | QFrame::Raised);
  //label4->setLineWidth(2); label5->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  //label5->setLineWidth(2);

  //QVBoxLayout *layout = new QVBoxLayout;
  //layout->addWidget(label); layout->addWidget(label1);
  //layout->addWidget(label2); layout->addWidget(label3);
  //layout->addWidget(label4); layout->addWidget(label5);
  //win->setLayout(layout);
  //win->show();

  application->exec();
}
