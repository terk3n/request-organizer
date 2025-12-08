#include "HttpSyntaxHighlighter.h"
#include <QColor>

HttpSyntaxHighlighter::HttpSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    // HTTP Method format (GET, POST, PUT, DELETE, etc.)
    methodFormat.setForeground(QColor(200, 50, 50));
    methodFormat.setFontWeight(QFont::Bold);
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b(GET|POST|PUT|DELETE|PATCH|HEAD|OPTIONS)\\b", QRegularExpression::CaseInsensitiveOption);
    rule.format = methodFormat;
    highlightingRules.append(rule);

    // HTTP Headers format
    headerFormat.setForeground(QColor(50, 150, 200));
    rule.pattern = QRegularExpression("^([A-Za-z-]+):\\s*");
    rule.format = headerFormat;
    highlightingRules.append(rule);

    // URL format
    urlFormat.setForeground(QColor(100, 150, 200));
    urlFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    rule.pattern = QRegularExpression("(https?://[^\\s]+|/[^\\s]*)");
    rule.format = urlFormat;
    highlightingRules.append(rule);

    // HTTP Status codes
    statusFormat.setForeground(QColor(200, 100, 50));
    statusFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b(HTTP/[0-9.]+\\s+[0-9]{3})\\b");
    rule.format = statusFormat;
    highlightingRules.append(rule);

    // JSON keys
    jsonKeyFormat.setForeground(QColor(150, 100, 200));
    rule.pattern = QRegularExpression("\"([^\"]+)\"\\s*:");
    rule.format = jsonKeyFormat;
    highlightingRules.append(rule);

    // JSON strings
    jsonStringFormat.setForeground(QColor(50, 200, 100));
    rule.pattern = QRegularExpression(":\\s*\"([^\"]*)\"");
    rule.format = jsonStringFormat;
    highlightingRules.append(rule);

    // JSON numbers
    jsonNumberFormat.setForeground(QColor(200, 150, 50));
    rule.pattern = QRegularExpression(":\\s*(-?\\d+\\.?\\d*)");
    rule.format = jsonNumberFormat;
    highlightingRules.append(rule);

    // Comments
    commentFormat.setForeground(QColor(100, 100, 100));
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//.*$");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

void HttpSyntaxHighlighter::highlightBlock(const QString& text) {
    for (const HighlightingRule& rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
