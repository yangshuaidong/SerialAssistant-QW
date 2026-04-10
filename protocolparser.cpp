#include "protocolparser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// ModbusParser implementation
bool ModbusParser::match(const QByteArray &data) const {
    // Modbus RTU frame: address (1) + function (1) + data (N) + CRC (2)
    // Minimum 4 bytes, maximum 256 bytes
    if (data.size() < 4 || data.size() > 256) {
        return false;
    }
    
    // Check CRC
    quint16 crc = calculateCRC(data.left(data.size() - 2));
    quint16 receivedCrc = static_cast<quint8>(data[data.size()-2]) |
                         (static_cast<quint8>(data[data.size()-1]) << 8);
    
    return crc == receivedCrc;
}

QVariantMap ModbusParser::parse(const QByteArray &data) const {
    QVariantMap result;
    if (data.size() < 4) {
        return result;
    }
    
    result["address"] = static_cast<quint8>(data[0]);
    result["function"] = static_cast<quint8>(data[1]);
    result["payload"] = data.mid(2, data.size() - 4);
    result["crc"] = QString("%1")
        .arg(static_cast<quint8>(data[data.size()-2]), 2, 16, QChar('0'))
        .arg(static_cast<quint8>(data[data.size()-1]), 2, 16, QChar('0'));
    
    return result;
}

QByteArray ModbusParser::build(const QVariantMap &data) const {
    QByteArray frame;
    frame.append(static_cast<char>(data["address"].toInt()));
    frame.append(static_cast<char>(data["function"].toInt()));
    
    if (data.contains("payload")) {
        frame.append(data["payload"].toByteArray());
    }
    
    // Calculate and append CRC
    quint16 crc = calculateCRC(frame);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    
    return frame;
}

quint16 ModbusParser::calculateCRC(const QByteArray &data) const {
    quint16 crc = 0xFFFF;
    
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

// JsonParser implementation
bool JsonParser::match(const QByteArray &data) const {
    if (data.isEmpty()) {
        return false;
    }
    
    // Quick check: should start with { or [
    char first = data.trimmed().at(0);
    return first == '{' || first == '[';
}

QVariantMap JsonParser::parse(const QByteArray &data) const {
    QVariantMap result;
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error == QJsonParseError::NoError) {
        if (doc.isObject()) {
            result = doc.object().toVariantMap();
        } else if (doc.isArray()) {
            result["array"] = doc.array().toVariantList();
        }
    }
    
    return result;
}

QByteArray JsonParser::build(const QVariantMap &data) const {
    QJsonObject obj;
    for (auto it = data.begin(); it != data.end(); ++it) {
        obj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

// CustomFrameParser implementation
CustomFrameParser::CustomFrameParser(const QString &name,
                                     const QByteArray &header,
                                     const QByteArray &footer)
    : m_name(name)
    , m_header(header)
    , m_footer(footer)
{
}

bool CustomFrameParser::match(const QByteArray &data) const {
    if (data.isEmpty()) {
        return false;
    }
    
    // Check header if specified
    if (!m_header.isEmpty()) {
        if (data.size() < m_header.size()) {
            return false;
        }
        if (data.left(m_header.size()) != m_header) {
            return false;
        }
    }
    
    // Check footer if specified
    if (!m_footer.isEmpty()) {
        if (data.size() < m_footer.size()) {
            return false;
        }
        if (data.right(m_footer.size()) != m_footer) {
            return false;
        }
    }
    
    return true;
}

QVariantMap CustomFrameParser::parse(const QByteArray &data) const {
    QVariantMap result;
    
    int start = 0;
    int end = data.size();
    
    // Skip header
    if (!m_header.isEmpty() && data.startsWith(m_header)) {
        start = m_header.size();
    }
    
    // Skip footer
    if (!m_footer.isEmpty() && data.endsWith(m_footer)) {
        end -= m_footer.size();
    }
    
    if (end > start) {
        result["payload"] = data.mid(start, end - start);
    }
    
    return result;
}

QByteArray CustomFrameParser::build(const QVariantMap &data) const {
    QByteArray frame;
    
    if (!m_header.isEmpty()) {
        frame.append(m_header);
    }
    
    if (data.contains("payload")) {
        frame.append(data["payload"].toByteArray());
    }
    
    if (!m_footer.isEmpty()) {
        frame.append(m_footer);
    }
    
    return frame;
}

void CustomFrameParser::setHeader(const QByteArray &header) {
    m_header = header;
}

void CustomFrameParser::setFooter(const QByteArray &footer) {
    m_footer = footer;
}

// ProtocolParserManager implementation
ProtocolParserManager* ProtocolParserManager::m_instance = nullptr;

ProtocolParserManager::ProtocolParserManager(QObject *parent)
    : QObject(parent)
{
    // Register built-in parsers
    registerParser(new ModbusParser());
    registerParser(new JsonParser());
}

ProtocolParserManager* ProtocolParserManager::instance() {
    if (!m_instance) {
        m_instance = new ProtocolParserManager();
    }
    return m_instance;
}

void ProtocolParserManager::registerParser(IProtocolParser *parser) {
    if (parser && !m_parsers.contains(parser)) {
        m_parsers.append(parser);
    }
}

void ProtocolParserManager::unregisterParser(IProtocolParser *parser) {
    m_parsers.removeAll(parser);
}

IProtocolParser* ProtocolParserManager::detectProtocol(const QByteArray &data) const {
    for (IProtocolParser *parser : m_parsers) {
        if (parser->match(data)) {
            return parser;
        }
    }
    return nullptr;
}

QVariantMap ProtocolParserManager::parseData(const QByteArray &data) const {
    IProtocolParser *parser = detectProtocol(data);
    if (parser) {
        return parser->parse(data);
    }
    return QVariantMap();
}

QList<QString> ProtocolParserManager::availableProtocols() const {
    QList<QString> protocols;
    for (IProtocolParser *parser : m_parsers) {
        protocols.append(parser->name());
    }
    return protocols;
}
