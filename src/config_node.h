#pragma once

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

  ConfigNode member(const QString &name) const;
  bool hasMember(const QString &name) const;
  ConfigNode memberNoExcept(const QString &name) const;
  ConfigNode element(unsigned index) const;
  size_t numElements() const;
  bool isScalar() const;
  QString asString() const;
  QStringList asStringList() const;

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
    catch (YAML::BadConversion) {
        return false;
    }
}

template <typename T> T ConfigNode::as() const
{
    if (!is<T>())
        throw error(HERE) << "is not a " << typeid(T).name();
    return node_.as<T>();
}
