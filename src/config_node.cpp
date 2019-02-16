#include "struct_config.h"

#include "log.h"

ConfigError::ConfigError(const QString &path, const LogContext &context)
    : Exception(context)
{
    QDEBUG_COMPAT(stream_.d);
    stream_ << "Error parsing config: " << path << " ";
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
        throw error(HERE) << "does not have member '" << name << "'";
    return member(name);
}

ConfigNode
ConfigNode::member(const QString &name, const QString &defaultYaml) const
{
    auto path = path_ + "/" + name;
    auto yamlNode = hasMember(name)
                        ? node_[name.toStdString()]
                        : (defaultYaml.isNull()
                               ? YAML::Node(YAML::NodeType::Undefined)
                               : (defaultYaml.isEmpty()
                                      ? YAML::Load("''")
                                      : YAML::Load(defaultYaml.toStdString())));
    return ConfigNode(path, yamlNode);
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
    return ConfigError(path_, context);
}

std::vector<std::pair<QString, ConfigNode>> ConfigNode::members() const
{
    assertIsObject();
    std::vector<std::pair<QString, ConfigNode>> result;
    for (const auto &node : node_) {
        auto key = QString::fromStdString(node.first.as<std::string>());
        result.emplace_back(key, ConfigNode(path_ + "/" + key, node.second));
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

