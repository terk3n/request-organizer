#ifndef HTTPSYNTAXHIGHLIGHTER_H
#define HTTPSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class HttpSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    HttpSyntaxHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat methodFormat;
    QTextCharFormat headerFormat;
    QTextCharFormat urlFormat;
    QTextCharFormat statusFormat;
    QTextCharFormat jsonKeyFormat;
    QTextCharFormat jsonStringFormat;
    QTextCharFormat jsonNumberFormat;
    QTextCharFormat commentFormat;
};

#endif // HTTPSYNTAXHIGHLIGHTER_H
