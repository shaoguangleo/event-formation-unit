/** Copyright (C) 2016 European Spallation Source */

#include <string>

class Detector;

class Loader {
private:
  void *handle{NULL};

public:
  Detector *detector{NULL};

  ~Loader();
  Loader(std::string lib);
};
