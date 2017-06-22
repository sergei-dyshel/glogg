// TODO: copyright

#include "cfg_file.h"
#include <stdio.h>

// #include <QTest>
#include <QJsonValue>

class TestConfig : public cfg::FileBacked {
public:
	CFG_INT(a, setA, 5);
	CFG_INT(b, setB, 6);

	CFG_DEFAULT_FROM_TO_JSON(a, b);
};

int main() {
	return 0;
}

