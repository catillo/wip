/* http://disease13.imascientist.org.au/2013/03/18/what-are-the-highest-and-lowest-known-pulses-heart-rate-recorded/ */
#define MAX_HEART_RATE 300

#define CORBOT_RESTING_HEART_RATE     60
#define CORBOT_INCREASED_HEART_RATE   120
#define CORBOT_CHECK_INTERVAL_MS      250

#define SECONDS_IN_1_MIN              60
#define MS_IN_1_SEC                   1000

#define BPM_ACCELERATION              5
#define SERIAL_BAUD_RATE              230400

#define TEMPERATURE_SENSOR_PIN 10
#define VIBRATION_SENSOR_PIN 11
#define POSITION_SENSOR_PIN 12
#define PRESSURE_SENSOR_PIN 13

#define PHASE_INITIALIZING      0
#define PHASE_SYSTOLE           1
#define PHASE_DIASTOLE          2


/****************************************************************/
/****************************************************************/
/************************* Declarations **************************/
/****************************************************************/
/****************************************************************/

template <class T>
class auto_ptr {
private:
    T* _t;
public:
    auto_ptr() : _t(0) {
    }

    auto_ptr(T* t) : _t(t) {
    }

    template<typename _T1>
    auto_ptr(auto_ptr<_T1> &__a) : _t(__a.release()) {
    }

    auto_ptr<T> operator=(auto_ptr<T>& p) {
        if (_t) {
            delete _t;
        }

        _t = p.release();
    }

    T* operator->() {
        return this->get();
    }

    ~auto_ptr() {
        if(_t) {
            delete _t;
            _t = NULL;
        }
    }

    T* get() const {
        return _t;
    }

    T* release() {
        T* tmp = _t;
        _t = NULL;

        return tmp;
    }
};


class Element {
public:
    Element* _next;
    Element();
    virtual ~Element();
};

class ListIterator;
class ListElem : public Element {
private:
    Element* _head;
    friend class ListIterator;
public:
    ListElem();
    ~ListElem();
    bool add(auto_ptr<Element>);
    auto_ptr<ListIterator> iterator();
};


class ListIterator {
private:
    ListElem *_owner;
    unsigned int _index;
public:
    ListIterator(ListElem *owner);
    Element* next();
    bool hasNext();

    ListIterator();
    ~ListIterator();
};

class TimerFunction : public Element {
private:
    unsigned long _interval;
    unsigned long _lastRun;
public:
    TimerFunction();
    virtual ~TimerFunction();
    void update(unsigned long elapsed_millis);
    virtual void run(unsigned long elapsed_millis) = 0;
    void setInterval(unsigned long interval);
};

class TimerSys {
private:
public:
    static ListElem _functions;
    static bool register_func(auto_ptr<TimerFunction> tf, unsigned long interval);
    static void update(unsigned long elapsed_millis);
};

class Heart : public TimerFunction {
private:
    static unsigned char _ledStates[];
    static unsigned char _ledPins[];


    unsigned long _lastRun;
    unsigned int _currLedState;
    int _numLedStates;
    unsigned long* _currTimeTable;
    unsigned long* _stateTimeTable;
    int _phase;
    unsigned int _currBpm;
    unsigned int _newBpm;
public:
    static void writeLed(unsigned char state);
    void updateTimeTable(unsigned long elapsed_millis);
    void updateStateTable();
    void updateBpm(unsigned int bpm);
    Heart(unsigned int bpm, unsigned long elapsed_millis);
    ~Heart();
    virtual void run(unsigned long elapsed_millis);
};

class InputPinReader : public Element {
private:
  int _pin;
public:
  int _high;
  InputPinReader(int pin);
  virtual ~InputPinReader();
  void run();
};

class TemperatureChecker : public InputPinReader {
private:
public:
  TemperatureChecker();
  ~TemperatureChecker();
};

class VibrationChecker : public InputPinReader {
private:
public:
  VibrationChecker();
  ~VibrationChecker();
};

class PositionChecker : public InputPinReader {
private:
public:
  PositionChecker();
  ~PositionChecker();
};

class PressureChecker : public InputPinReader {
private:
public:
  PressureChecker();
  ~PressureChecker();
};

class Corbot : public TimerFunction {
private:
  Heart* _heart;
  int _currBpm;
  int _minBpm;
  int _maxBpm;
  auto_ptr<ListElem> _readers;
public:
  Corbot(Heart* heart, auto_ptr<ListElem> readers);
  ~Corbot();
  void run(unsigned long elapsed_millis);
};

ListElem TimerSys::_functions;
unsigned char Heart::_ledStates[] = {
    0x01,
    0x02,
    0x04,
    0x08,
    0x10,
    0x20,
    0x40,
    0x80
};

/****************************************************************/
/****************************************************************/
/************************* Definitions **************************/
/****************************************************************/
/****************************************************************/

unsigned char Heart::_ledPins[] = {2,3,4,5,6,7,8,9};

unsigned long round_closest_divide(unsigned long dividend, unsigned long divisor)
{
    return (dividend + (divisor / 2)) / divisor;
}

Element::Element() : _next(0) {}
Element::~Element() {}

ListElem::ListElem() : _head(0) {}

ListElem::~ListElem() {
    while (_head != NULL) {
        Element* tmp = _head;
        _head = _head->_next;

        delete tmp;
    }
}

bool ListElem::add(auto_ptr<Element> e) {
    if (e.get() == NULL) {
        return false;
    }

    if (_head == NULL) {
        _head = e.release();
        _head->_next = NULL;
    } else {
        Element* runner = _head;
        while (runner->_next != NULL) {
            runner = runner->_next;
        }

        runner->_next = e.release();
        runner->_next->_next = NULL;
    }

    return true;
}

auto_ptr<ListIterator> ListElem::iterator() {
    auto_ptr<ListIterator> r (new ListIterator(this));

    return r;
}

ListIterator::ListIterator(ListElem* owner) : _owner(owner), _index(0) {}
ListIterator::~ListIterator() {}

Element* ListIterator::next() {
    Element *runner = _owner->_head;
    unsigned int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            _index++;
            return runner;
        }

        i++;
        runner = runner->_next;
    }

    return NULL;
}

bool ListIterator::hasNext() {
    Element *runner = _owner->_head;
    unsigned int i = 0;
    while (runner != NULL) {
        if (i == _index) {
            return true;
        }

        i++;
        runner = runner->_next;
    }

    return false;
}

void TimerFunction::setInterval(unsigned long interval) {
    _interval = interval;
}

TimerFunction::TimerFunction() : _interval(0), _lastRun(0) {}
TimerFunction::~TimerFunction() {}

void TimerFunction::update(unsigned long elapsed_millis) {
    if (_interval == 0) {
      run(elapsed_millis);
    } else {
      if (_lastRun == 0) {
          _lastRun = elapsed_millis;
          return;
      } else {
          if ((elapsed_millis - _lastRun) > _interval) {
              _lastRun = elapsed_millis;
              run(elapsed_millis);
          }
      }
    }
}

bool TimerSys::register_func(auto_ptr<TimerFunction> tf, unsigned long interval) {
    tf->setInterval(interval);
    auto_ptr<Element> t (tf.release());
    return TimerSys::_functions.add(t);
}

void TimerSys::update(unsigned long elapsed_millis) {
    auto_ptr<ListIterator> i = TimerSys::_functions.iterator();

    while (i->hasNext()) {
        TimerFunction* tf = static_cast<TimerFunction*>(i->next());
        tf->update(elapsed_millis);
    }
}

void printTimeTable(unsigned long* pTimeTable, int size) {
#if defined(DEBUG_ON)
    Serial.print("\n");
    for (int i=0; i < size; i++) {
        Serial.print("timeTable[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.print(*pTimeTable++);
        Serial.print("\n");
    }
#endif
}

void Heart::updateTimeTable(unsigned long elapsed_millis) {
    int size = _numLedStates + 1;

    for (int i = 0; i < size; i++) {
        _currTimeTable[i] = elapsed_millis + _stateTimeTable[i];
    }
    printTimeTable(_currTimeTable, size);
}

void Heart::writeLed(unsigned char state) {
  unsigned char bitMask = 0x01;

  for(int i=0; i < sizeof(Heart::_ledPins); i++) {
    if(state & bitMask) {
      digitalWrite(Heart::_ledPins[i], HIGH);
    } else {
      digitalWrite(Heart::_ledPins[i], LOW);
    }

    bitMask <<= 1;
  }
}

Heart::Heart(unsigned int bpm, unsigned long elapsed_millis) :
    TimerFunction(), _lastRun(elapsed_millis), _currLedState(0),
    _numLedStates(0), _currTimeTable(0), _stateTimeTable(0), _phase(PHASE_SYSTOLE) {

    for(int i = 0; i < sizeof(Heart::_ledPins); i++){
      pinMode(Heart::_ledPins[i], OUTPUT);
    }

    _numLedStates = sizeof(Heart::_ledStates);
    if (_numLedStates <= 0) {
        return;
    }

    _currBpm = bpm;
    _newBpm = bpm;

    _currTimeTable = new unsigned long[_numLedStates + 1];
    _stateTimeTable = new unsigned long[_numLedStates + 1];

    updateStateTable();
    updateTimeTable(elapsed_millis);
}

void Heart::updateStateTable() {
  int bpm = _currBpm;

  unsigned long ms_per_cycle = ((unsigned long) SECONDS_IN_1_MIN * MS_IN_1_SEC) / bpm;

  int cycle_divisor = _numLedStates * 3;
  for (int i=0; i < _numLedStates; i++) {
      _stateTimeTable[i] = (round_closest_divide(ms_per_cycle, cycle_divisor) * i);
  }
  _stateTimeTable[_numLedStates] = ms_per_cycle;

  printTimeTable(_stateTimeTable, _numLedStates + 1);
}

Heart::~Heart() {
    if (_stateTimeTable) {
        delete [] _stateTimeTable;
        _stateTimeTable = NULL;
    }

    if (_currTimeTable) {
        delete [] _currTimeTable;
        _currTimeTable = NULL;
    }
}

void Heart::updateBpm(unsigned int bpm) {
  if(bpm == _currBpm) {
    return;
  }

  if(bpm > MAX_HEART_RATE) {
    return;
  }

  _newBpm = bpm;
}

void Heart::run(unsigned long elapsed_millis) {
    int ledState = 0;
    if (_numLedStates <= 0) {
        return;
    }
    static unsigned long numBeats = 0;

    if (_phase == PHASE_SYSTOLE) {
        if (_currLedState >= _numLedStates) {
            _phase = PHASE_DIASTOLE;
        } else {
            unsigned long nxt_state = _currTimeTable[_currLedState];
            if (elapsed_millis >= nxt_state) {
                ledState = _ledStates[_currLedState];
                Heart::writeLed(ledState);
                _currLedState++;
            }
        }
    } else if (_phase == PHASE_DIASTOLE) {
        unsigned long nxt_state = _currTimeTable[_numLedStates];
        if (elapsed_millis >= nxt_state) {
            _phase = PHASE_SYSTOLE;
            _currLedState = 0;

            if(_newBpm != _currBpm) {
                _currBpm = _newBpm;
                updateStateTable();
            }

            updateTimeTable(elapsed_millis);

            numBeats++;

            Serial.print("numBeats = ");
            Serial.print(numBeats);
            Serial.print("\n");
        }
    }
}

InputPinReader::InputPinReader(int pin) : _pin(pin), _high(false) {
  pinMode(_pin, INPUT);
}

InputPinReader::~InputPinReader() {}

void InputPinReader::run() {
  if (digitalRead(_pin) == HIGH) {
    _high = true;
  } else {
    _high = false;
  }
}

TemperatureChecker::TemperatureChecker() : InputPinReader(TEMPERATURE_SENSOR_PIN) {}
TemperatureChecker::~TemperatureChecker() {}

VibrationChecker::VibrationChecker() : InputPinReader(VIBRATION_SENSOR_PIN) {}
VibrationChecker::~VibrationChecker() {}

PositionChecker::PositionChecker() : InputPinReader(POSITION_SENSOR_PIN) {}
PositionChecker::~PositionChecker() {}

PressureChecker::PressureChecker() : InputPinReader(PRESSURE_SENSOR_PIN) {}
PressureChecker::~PressureChecker() {}

Corbot::Corbot(Heart* heart, auto_ptr<ListElem> readers) :
  _heart(heart),
  _currBpm(CORBOT_RESTING_HEART_RATE), _minBpm(CORBOT_RESTING_HEART_RATE), _maxBpm(CORBOT_INCREASED_HEART_RATE), _readers(readers) {
    _heart->updateBpm(_currBpm);
}

Corbot::~Corbot() {}

void Corbot::run(unsigned long elapsed_millis) {
  bool increaseHeartRate = false;

  auto_ptr<ListIterator> i(_readers->iterator());
  while(i->hasNext()) {
    InputPinReader* r = static_cast<InputPinReader*>(i->next());
    r->run();
    if(r->_high) {
      increaseHeartRate = true;
    }
  }

  if(increaseHeartRate) {
    int newBpm = _currBpm + BPM_ACCELERATION;

    _currBpm = newBpm > _maxBpm ? _maxBpm:newBpm;
  } else {
    int newBpm = _currBpm - BPM_ACCELERATION;

    _currBpm = newBpm < _minBpm ? _minBpm:newBpm;
  }

  _heart->updateBpm(_currBpm);

  Serial.print("bpm = ");
  Serial.print(_currBpm);
  Serial.print("\n");
}

void setup(void) {
  Serial.begin(SERIAL_BAUD_RATE);
  unsigned long elapsed_millis = millis();

  auto_ptr<TimerFunction> heart(new Heart(CORBOT_RESTING_HEART_RATE, elapsed_millis));

  auto_ptr<InputPinReader> tempChecker(new TemperatureChecker());
  auto_ptr<InputPinReader> vibrationChecker(new VibrationChecker());
  auto_ptr<InputPinReader> positionChecker(new PositionChecker());
  auto_ptr<InputPinReader> pressureChecker(new PressureChecker());

  auto_ptr<ListElem> readers(new ListElem());
  readers->add(tempChecker);
  readers->add(vibrationChecker);
  readers->add(positionChecker);
  readers->add(pressureChecker);

  auto_ptr<TimerFunction> corbot(new Corbot(static_cast<Heart*>(heart.get()),
                                                                    readers));

  TimerSys::register_func(heart, 0);
  TimerSys::register_func(corbot, CORBOT_CHECK_INTERVAL_MS);
}

void loop(void) {
  unsigned long elapsed_millis = millis();

  TimerSys::update(elapsed_millis);
}
