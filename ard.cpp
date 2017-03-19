/* http://disease13.imascientist.org.au/2013/03/18/what-are-the-highest-and-lowest-known-pulses-heart-rate-recorded/ */
#define MAX_HEART_RATE 300

#define BPM_ACCELERATION 5

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

class UpdateLED : public TimerFunction {
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
    UpdateLED(unsigned int bpm, unsigned long elapsed_millis);
    ~UpdateLED();
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

class CheckTemperature : public InputPinReader {
private:
public:
  CheckTemperature();
  ~CheckTemperature();
};

class CheckVibration : public InputPinReader {
private:
public:
  CheckVibration();
  ~CheckVibration();
};

class CheckPosition : public InputPinReader {
private:
public:
  CheckPosition();
  ~CheckPosition();
};

class CheckPressure : public InputPinReader {
private:
public:
  CheckPressure();
  ~CheckPressure();
};

class Corbot : public TimerFunction {
private:
  UpdateLED* _updateLed;
  int _currBpm;
  int _minBpm;
  int _maxBpm;
  auto_ptr<ListElem> _readers;
public:
  Corbot(UpdateLED* updateLed, auto_ptr<ListElem> readers);
  ~Corbot();
  void run(unsigned long elapsed_millis);
};

ListElem TimerSys::_functions;
unsigned char UpdateLED::_ledStates[] = {
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

unsigned char UpdateLED::_ledPins[] = {2,3,4,5,6,7,8,9};

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

void UpdateLED::updateTimeTable(unsigned long elapsed_millis) {
    int size = _numLedStates + 1;

    for (int i = 0; i < size; i++) {
        _currTimeTable[i] = elapsed_millis + _stateTimeTable[i];
    }
    printTimeTable(_currTimeTable, size);
}

void UpdateLED::writeLed(unsigned char state) {
  unsigned char bitMask = 0x01;

  for(int i=0; i < sizeof(UpdateLED::_ledPins); i++) {
    if(state & bitMask) {
      digitalWrite(UpdateLED::_ledPins[i], HIGH);
    } else {
      digitalWrite(UpdateLED::_ledPins[i], LOW);
    }

    bitMask <<= 1;
  }
}

UpdateLED::UpdateLED(unsigned int bpm, unsigned long elapsed_millis) :
    TimerFunction(), _lastRun(elapsed_millis), _currLedState(0),
    _numLedStates(0), _currTimeTable(0), _stateTimeTable(0), _phase(PHASE_SYSTOLE) {

    for(int i = 0; i < sizeof(UpdateLED::_ledPins); i++){
      pinMode(UpdateLED::_ledPins[i], OUTPUT);
    }

    _numLedStates = sizeof(UpdateLED::_ledStates);
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

void UpdateLED::updateStateTable() {
  int bpm = _currBpm;

  unsigned long ms_per_cycle = ((unsigned long) 60 * 1000) / bpm;

  int cycle_divisor = _numLedStates * 3;
  for (int i=0; i < _numLedStates; i++) {
      _stateTimeTable[i] = (round_closest_divide(ms_per_cycle, cycle_divisor) * i);
  }
  _stateTimeTable[_numLedStates] = ms_per_cycle;

  printTimeTable(_stateTimeTable, _numLedStates + 1);
}

UpdateLED::~UpdateLED() {
    if (_stateTimeTable) {
        delete [] _stateTimeTable;
        _stateTimeTable = NULL;
    }

    if (_currTimeTable) {
        delete [] _currTimeTable;
        _currTimeTable = NULL;
    }
}

void UpdateLED::updateBpm(unsigned int bpm) {
  if(bpm == _currBpm) {
    return;
  }

  if(bpm > MAX_HEART_RATE) {
    return;
  }

  _newBpm = bpm;
}

void UpdateLED::run(unsigned long elapsed_millis) {
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
                UpdateLED::writeLed(ledState);
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

CheckTemperature::CheckTemperature() : InputPinReader(TEMPERATURE_SENSOR_PIN) {}
CheckTemperature::~CheckTemperature() {}

CheckVibration::CheckVibration() : InputPinReader(VIBRATION_SENSOR_PIN) {}
CheckVibration::~CheckVibration() {}

CheckPosition::CheckPosition() : InputPinReader(POSITION_SENSOR_PIN) {}
CheckPosition::~CheckPosition() {}

CheckPressure::CheckPressure() : InputPinReader(PRESSURE_SENSOR_PIN) {}
CheckPressure::~CheckPressure() {}



Corbot::Corbot(UpdateLED* updateLed, auto_ptr<ListElem> readers) :
  _updateLed(updateLed),
  _currBpm(60), _minBpm(60), _maxBpm(120), _readers(readers) {
    _updateLed->updateBpm(_currBpm);
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

  Serial.print("bpm = ");
  Serial.print(_currBpm);
  Serial.print("\n");
  _updateLed->updateBpm(_currBpm);
}

void setup(void) {
  Serial.begin(230400);
  unsigned long elapsed_millis = millis();

  auto_ptr<TimerFunction> updateLed(new UpdateLED(60, elapsed_millis));

  auto_ptr<InputPinReader> checkTemperature(new CheckTemperature());
  auto_ptr<InputPinReader> checkVibration(new CheckVibration());
  auto_ptr<InputPinReader> checkPosition(new CheckPosition());
  auto_ptr<InputPinReader> checkPressure(new CheckPressure());

  auto_ptr<ListElem> readers(new ListElem());
  readers->add(checkTemperature);
  readers->add(checkVibration);
  readers->add(checkPosition);
  readers->add(checkPressure);

  auto_ptr<TimerFunction> corbot(new Corbot(static_cast<UpdateLED*>(updateLed.get()),
                                                                    readers));

  TimerSys::register_func(updateLed, 0);
  TimerSys::register_func(corbot, 250);
}

void loop(void) {
  unsigned long elapsed_millis = millis();

  TimerSys::update(elapsed_millis);
}
