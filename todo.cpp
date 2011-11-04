#include "todo.h"
#include "ui_todo.h"
#include "ui_settings.h"

ToDo::ToDo(QWidget *parent)
  : QMainWindow(parent), trayIcon(new QSystemTrayIcon(this)), ui(new Ui::ToDo), contextMenu(new QMenu(this))
{
  // Load UI from QT UI file.
  ui->setupUi(this);

  // Create context menu,
  createContextMenu();

  // Context menu after right click.
  connect(ui->diaryTextEdit, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(mainMenu(const QPoint &)));
  connect(ui->notesTextEdit, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(mainMenu(const QPoint &)));
  connect(ui->calendarWidget, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(mainMenu(const QPoint &)));

  // Text edits refreshing, after changes.
  connect(ui->diaryTextEdit, SIGNAL(textChanged()), this, SLOT(reload()));
  connect(ui->notesTextEdit, SIGNAL(textChanged()), this, SLOT(reload()));

  // Tray icon action.
  connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

  // Load resources.
  QResource::registerResource("/icons.qrc");

  // Set tray icon properties.
  trayIcon->setContextMenu(contextMenu);
  trayIcon->setIcon(QIcon(":/icon"));
  setWindowIcon(QIcon(":/icon"));

   // Load configuration from file.
  loadConfig();

  // Show all informations in calendar and textedits.
  emit reload();
}

ToDo::~ToDo()
{
  delete ui;
}

void ToDo::createContextMenu()
{
  contextMenu->addAction(QString::fromUtf8("Nastavení"), this, SLOT(settingsDialog()));
}

void ToDo::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason)
  {
    // Click or double click.
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::MiddleClick:
      if(this->isVisible())
      { // Hide open window.
        this->hide();
      }
      else
      { // Show minimized window.
        this->show();
      }
      break;
    default:
      break;
  }
}

void ToDo::closeEvent(QCloseEvent *event)
{
  if (trayIcon->isVisible())
  { // Hide in tray.
    hide();

    event->ignore();
  }
  else
  { // Save config and close.
    saveConfig();

    QMainWindow::closeEvent(event);
  }
}

void ToDo::changeEvent(QEvent *event)
{
  QMainWindow::changeEvent(event);
  if(event->type() == QEvent::WindowStateChange)
  {
    if(isMinimized() && trayIcon->isVisible())
    { // Minimized window hide to tray.
      this->hide();
    }
  }
}

void ToDo::loadConfig()
{
  // Load window setting.
  if (settings.value("general/tray", true).value<bool>())
  {
    trayIcon->show();
  }
  decorations = settings.value("general/decorations", false).value<bool>();
  resize(settings.value("general/size", QSize(250, 550)).value<QSize>());
  move(settings.value("general/position", QPoint(0, 0)).value<QPoint>());

  // Load format date and time.
  dateFormat = settings.value("format/date").value<QString>();
  timeFormat = settings.value("format/time").value<QString>();

  int size = settings.beginReadArray("colors");
  for (int i = 0; i < size; ++i)
  { // Load colors vector.
    settings.setArrayIndex(i);
    colors[i] = settings.value("color").value<QColor>();
  }
  settings.endArray();

  size = settings.beginReadArray("data");
  for (int i = 0; i < size; ++i)
  { // Load all data.
    settings.setArrayIndex(i);
    data[i].date = settings.value("color").value<QDate>();
    data[i].time = settings.value("color").value<QTime>();
    data[i].color = settings.value("color").value<int>();
    data[i].content = settings.value("color").value<QString>();
  }
  settings.endArray();

}

void ToDo::saveConfig()
{
  // Save window setting.
  settings.beginGroup("general");
  settings.setValue("tray", trayIcon->isVisible());
  settings.setValue("decorations", decorations);
  settings.setValue("position", pos());
  settings.setValue("size", size());
  settings.endGroup();

  // Save format date and time.
  settings.beginGroup("formats");
  settings.setValue("date", dateFormat);
  settings.setValue("time", timeFormat);
  settings.endGroup();

  settings.beginWriteArray("colors");
  for (int i = 0; i < colors.size(); ++i)
  { // Save colors vector.
    settings.setArrayIndex(i);
    settings.setValue("color", colors.at(i));
  }
  settings.endArray();

  settings.beginWriteArray("data");
  for (int i = 0; i < data.size(); ++i)
  { // Save all data.
    settings.setArrayIndex(i);
    settings.setValue("date", data.at(i).date);
    settings.setValue("time", data.at(i).time);
    settings.setValue("color", data.at(i).color);
    settings.setValue("content", data.at(i).content);
  }
  settings.endArray();
}

void ToDo::parse()
{
  QString row;
  // Create cursor for diary textEdit.
  QTextCursor *cursor = new QTextCursor(ui->diaryTextEdit->document());
  // Move cursor to start
  cursor->setPosition(0);
}

void ToDo::display()
{

}

void ToDo::reload()
{
  // Disable signals to avoid recursion.
  disconnect(ui->notesTextEdit, SIGNAL(textChanged()), this, SLOT(reload()));
  disconnect(ui->diaryTextEdit, SIGNAL(textChanged()), this, SLOT(reload()));

  // Parse text and save it to our data structures.
  parse();
  // Display changes in textArea.
  display();

  // Reenable signals.
  connect(ui->notesTextEdit, SIGNAL(textChanged()), this, SLOT(reload()));
  connect(ui->diaryTextEdit, SIGNAL(textChanged()), this, SLOT(reload()));
}

void ToDo::mainMenu(const QPoint &)
{
  // Show menu in point.
  contextMenu->popup(QCursor::pos());
  contextMenu->exec();
}

void ToDo::settingsDialog()
{
  // Open new window ui.
  Ui::Settings ui;
  QDialog *dialog = new QDialog;

  // Show window.
  ui.setupUi(dialog);
  ////// TODO: load settings
  dialog->exec();

  if (dialog->result() == QDialog::Accepted)
  { // Apply changes.

    ////// TODO: apply changes from form

    saveConfig();
    emit reload();
  }

  delete dialog;
  dialog = NULL;
}
