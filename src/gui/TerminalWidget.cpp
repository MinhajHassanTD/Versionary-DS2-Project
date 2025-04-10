#include "TerminalWidget.h"
#include <QKeyEvent>

TerminalWidget::TerminalWidget(VersionaryCLI* cli, QWidget* parent)
    : QWidget(parent), cli(cli), historyIndex(-1)
{
    setupUI();
    connectSignals();
}

void TerminalWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    outputDisplay = new QTextEdit(this);
    outputDisplay->setReadOnly(true);
    outputDisplay->setStyleSheet("background-color: #1E1E1E; color: #FFFFFF; font-family: Consolas, monospace;");
    
    commandInput = new QLineEdit(this);
    commandInput->setStyleSheet("background-color: #2D2D2D; color: #FFFFFF; font-family: Consolas, monospace;");
    commandInput->setPlaceholderText("Enter command...");
    
    layout->addWidget(outputDisplay);
    layout->addWidget(commandInput);
    
    setLayout(layout);
    
    // Initial welcome message
    appendOutput("Versionary Terminal\nType 'help' for available commands.\n");
}

void TerminalWidget::connectSignals() {
    connect(commandInput, &QLineEdit::returnPressed, this, &TerminalWidget::onCommandEntered);
    
    // Install event filter for up/down arrow keys
    commandInput->installEventFilter(this);
}

void TerminalWidget::appendOutput(const QString& text) {
    outputDisplay->append(text);
    // Scroll to bottom
    QTextCursor cursor = outputDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    outputDisplay->setTextCursor(cursor);
}

void TerminalWidget::executeCommand(const QString& command) {
    if (command.isEmpty()) {
        return;
    }
    
    // Add command to history
    commandHistory.prepend(command);
    historyIndex = -1;
    
    // Limit history size
    if (commandHistory.size() > 50) {
        commandHistory.removeLast();
    }
    
    // Display command
    appendOutput("> " + command);
    
    // Execute command
    std::string stdCommand = command.toStdString();
    bool success = cli->executeCommand(stdCommand);
    
    // Signal that command was executed
    emit commandExecuted(command, success);
}

void TerminalWidget::onCommandEntered() {
    QString command = commandInput->text();
    commandInput->clear();
    executeCommand(command);
}

bool TerminalWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == commandInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        
        if (keyEvent->key() == Qt::Key_Up) {
            // Navigate up in history
            if (!commandHistory.isEmpty() && historyIndex < commandHistory.size() - 1) {
                historyIndex++;
                commandInput->setText(commandHistory[historyIndex]);
            }
            return true;
        } else if (keyEvent->key() == Qt::Key_Down) {
            // Navigate down in history
            if (historyIndex > 0) {
                historyIndex--;
                commandInput->setText(commandHistory[historyIndex]);
            } else if (historyIndex == 0) {
                historyIndex = -1;
                commandInput->clear();
            }
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}