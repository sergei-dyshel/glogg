#pragma once

#include "enum.h"
#include "exception.h"
#include "yaml-cpp/yaml.h"

#include <string>
#include <typeinfo>
#include <vector>

#include <QString>
#include <QStringList>

class ConfigError : public Exception {
  public:
    ConfigError(const QString &path, const LogContext &context);
    DEFINE_EXCEPTION_SHIFT_OPERATOR(ConfigError)
};

class ConfigNode final {
public:
  ConfigNode() = default;
  ConfigNode(const QString &path, const YAML::Node &node)
      : path_(path), node_(node)
  {}

  ConfigNode(const QString &path);

  ConfigNode requiredMember(const QString &name) const;
  bool hasMember(const QString &name) const;
  ConfigNode member(const QString &name,
                    const QString &defaultYaml = QString()) const;
  ConfigNode element(unsigned index) const;
  size_t numElements() const;
  bool isScalar() const;
  QString asString() const;
  QStringList asStringList() const;
  bool isArray() const;
  bool isObject() const;

  template <typename E>
  E asEnum() const;

  template <typename E>
  E memberEnum(const QString &name,
               const E &defaultVal) const;

  std::vector<std::pair<QString, ConfigNode>> members() const;
  std::vector<ConfigNode> elements() const;

  template <typename T> bool is() const;
  template <typename T> T as() const;

  ConfigError error(const LogContext &context) const;

  operator bool() const { return node_.IsDefined(); }

private:
    void assertIsArray() const;
    void assertIsObject() const;

    QString path_;
    YAML::Node node_;
};

template <typename T> bool ConfigNode::is() const
{
    try {
        node_.as<T>();
        return true;
    }
    catch (const YAML::BadConversion &) {
        return false;
    }
}

template <typename T> T ConfigNode::as() const
{
    if (!is<T>())
        throw error(HERE) << "is not a " << typeid(T).name();
    return node_.as<T>();
}

template <typename E>
E ConfigNode::asEnum() const
{
    auto str = asString();
    auto strlist = Enum<E>::stringList();
    if (!strlist.count(str)) {
        throw error(HERE) << "is not one of" << strlist;
    }
    return Enum<E>::fromString(str);
}

template <typename E>
E ConfigNode::memberEnum(const QString &name, const E &defaultVal) const
{
    return hasMember(name) ? member(name).asEnum<E>() : defaultVal;
}
