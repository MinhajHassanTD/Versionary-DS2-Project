#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStringList>
#include "../cli/VersionaryCLI.h"

class TerminalWidget : public QWidget {
    Q_OBJECT
    
private:
    QTextEdit* outputDisplay;
    QLineEdit* commandInput;
    VersionaryCLI* cli;
    
    QStringList commandHistory;
    int historyIndex;
    
    void setupUI();
    void connectSignals();
    
private slots:
    void onCommandEntered();
    
public:
    TerminalWidget(VersionaryCLI* cli, QWidget* parent = nullptr);
    
    void appendOutput(const QString& text);
    void executeCommand(const QString& command);
    
    bool eventFilter(QObject* obj, QEvent* event) override;
    
signals:
    void commandExecuted(const QString& command, bool success);
};