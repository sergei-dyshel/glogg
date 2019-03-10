#include "struct_config.h"

#include "log.h"

ConfigNode::ConfigNode(const YAML::Node &node, const QString &path)
    : node_(node), path_(path), location_(path, node.Mark().line)
{}

ConfigError::ConfigError(const Location &location, const LogContext &context)
    : Exception(context)
{
    QDEBUG_COMPAT(stream_.d);
    stream_ << location << ":";
}

ConfigNode ConfigNode::parseFile(const QString &file)
{
    return ConfigNode(YAML::LoadFile(file.toStdString()), file);
}
ConfigNode ConfigNode::parseString(const QString &str)
{
    return ConfigNode(YAML::Load(str.toStdString()));
}

bool ConfigNode::isArray() const
{
    return node_.IsSequence();
}

bool ConfigNode::isObject() const
{
    return node_.IsMap();
}

void ConfigNode::assertIsArray() const
{
    if (!isArray())
        throw error(HERE) << " is not an array";
}

void ConfigNode::assertIsObject() const
{
    if (!isObject())
        throw error(HERE) << " is not an object";
}

bool ConfigNode::hasMember(const QString &name) const
{
    return node_[name.toStdString()];
}

ConfigNode ConfigNode::requiredMember(const QString &name) const
{
    assertIsObject();
    if (!hasMember(name))
        throw error(HERE) << "does not have member" << name;
    return member(name);
}

ConfigNode
ConfigNode::member(const QString &name, const QString &defaultYaml) const
{
    auto yamlNode = hasMember(name)
                        ? node_[name.toStdString()]
                        : (defaultYaml.isNull()
                               ? YAML::Node(YAML::NodeType::Undefined)
                               : (defaultYaml.isEmpty()
                                      ? YAML::Load("''")
                                      : YAML::Load(defaultYaml.toStdString())));
    return ConfigNode(yamlNode, path_);
}

ConfigNode ConfigNode::element(unsigned index) const
{
    assertIsArray();
    return ConfigNode(node_[index], path_);
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
    } catch (const YAML::BadConversion &) {
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
    return ConfigError(location_, context);
}

std::map<QString, ConfigNode> ConfigNode::members() const
{
    assertIsObject();
    std::map<QString, ConfigNode> result;
    for (const auto &node : node_) {
        auto key = QString::fromStdString(node.first.as<std::string>());
        result.emplace(key, ConfigNode(node.second, path_));
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

QString ConfigNode::toString() const
{
    return QString::fromStdString(YAML::Dump(node_));
}