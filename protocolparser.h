#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QList>

// Interface for protocol parsers
class IProtocolParser {
public:
    virtual ~IProtocolParser() = default;
    
    // Check if data matches this protocol
    virtual bool match(const QByteArray &data) const = 0;
    
    // Parse data into structured format
    virtual QVariantMap parse(const QByteArray &data) const = 0;
    
    // Build data from structured format
    virtual QByteArray build(const QVariantMap &data) const = 0;
    
    // Protocol name
    virtual QString name() const = 0;
};

// Modbus RTU Parser
class ModbusParser : public IProtocolParser {
public:
    bool match(const QByteArray &data) const override;
    QVariantMap parse(const QByteArray &data) const override;
    QByteArray build(const QVariantMap &data) const override;
    QString name() const override { return "Modbus RTU"; }
    
private:
    quint16 calculateCRC(const QByteArray &data) const;
};

// JSON Parser
class JsonParser : public IProtocolParser {
public:
    bool match(const QByteArray &data) const override;
    QVariantMap parse(const QByteArray &data) const override;
    QByteArray build(const QVariantMap &data) const override;
    QString name() const override { return "JSON"; }
};

// Custom frame parser (configurable)
class CustomFrameParser : public IProtocolParser {
public:
    explicit CustomFrameParser(const QString &name, 
                               const QByteArray &header = QByteArray(),
                               const QByteArray &footer = QByteArray());
    
    bool match(const QByteArray &data) const override;
    QVariantMap parse(const QByteArray &data) const override;
    QByteArray build(const QVariantMap &data) const override;
    QString name() const override { return m_name; }
    
    void setHeader(const QByteArray &header);
    void setFooter(const QByteArray &footer);
    
private:
    QString m_name;
    QByteArray m_header;
    QByteArray m_footer;
};

// Protocol Parser Manager
class ProtocolParserManager : public QObject {
    Q_OBJECT
    
public:
    static ProtocolParserManager* instance();
    
    void registerParser(IProtocolParser *parser);
    void unregisterParser(IProtocolParser *parser);
    
    IProtocolParser* detectProtocol(const QByteArray &data) const;
    QVariantMap parseData(const QByteArray &data) const;
    
    QList<QString> availableProtocols() const;

private:
    ProtocolParserManager(QObject *parent = nullptr);
    static ProtocolParserManager *m_instance;
    
    QList<IProtocolParser*> m_parsers;
};

#endif // PROTOCOLPARSER_H
