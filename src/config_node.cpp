#include "struct_config.h"

#include "log.h"

ConfigError::ConfigError(const QString &path, const LogContext &context)
    : Exception(context)
{
    QDEBUG_COMPAT(stream_.d);
    stream_ << "Error parsing config: " << path << " ";
}

void ConfigNode::assertIsArray() const
{
    if (!node_.IsSequence())
        throw error(HERE) << " is not an array";
}

void ConfigNode::assertIsObject() const
{
    if (!node_.IsMap())
        throw error(HERE) << " is not an object";
}

bool ConfigNode::hasMember(const QString &name) const
{
    return memberNoExcept(name);
}

ConfigNode ConfigNode::member(const QString &name) const
{
    assertIsObject();
    if (!hasMember(name))
        throw error(HERE) << "does not have member '" << name << "'";
    return memberNoExcept(name);
}

ConfigNode ConfigNode::memberNoExcept(const QString &name) const
{
    return ConfigNode(path_ + "/" + name, node_[name.toStdString()]);
}

ConfigNode ConfigNode::element(unsigned index) const
{
    assertIsArray();
    return ConfigNode(path_ + "[" + QString::number(index) + "]", node_[index]);
}

size_t ConfigNode::numElements() const
{
    assertIsArray();
    return node_.size();
}

bool ConfigNode::isScalar() const
{
    return node_.IsScalar();
}

QString ConfigNode::asString() const
{
    if (!node_.IsScalar())
        throw error(HERE) << "is not scalar";
    try {
        return QString::fromStdString(node_.as<std::string>());
    }
    catch (YAML::BadConversion) {
        throw error(HERE) << "is not a string";
    }
}

QStringList ConfigNode::asStringList() const
{
    QStringList result;
    for (const auto &elem : elements())
        result.push_back(elem.asString());
    return result;
}


ConfigError ConfigNode::error(const LogContext &context) const
{
    return ConfigError(path_, context);
}

std::vector<std::pair<QString, ConfigNode>> ConfigNode::members() const
{
    assertIsObject();
    std::vector<std::pair<QString, ConfigNode>> result;
    for (const auto &node : node_) {
        auto key = QString::fromStdString(node.first.as<std::string>());
        result.emplace_back(key, ConfigNode(key, node.second));
    }
    return result;
}

std::vector<ConfigNode> ConfigNode::elements() const
{
    std::vector<ConfigNode> result;
    for (unsigned i = 0; i < numElements(); ++i)
        result.push_back(element(i));
    return result;
}

