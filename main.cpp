#include <iostream>
#include <list>
#include <math.h>
#include <fstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>
#include <string>
#include <functional>
using namespace std;
using namespace std::chrono;    




// TODO - should be in separate file
/**
  String representation of current time.
  (from the internet)
*/
string now() {

    // get current time
    auto now = system_clock::now();

    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    tm bt = *localtime(&timer);

    ostringstream oss;

    oss << put_time(&bt, "%H:%M:%S"); // HH:MM:SS
    oss << '.' << setfill('0') << setw(3) << ms.count();

    return oss.str();
};

/**
  Parse csv line

 (from Stackover flow https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c)
*/
vector<string> getNextLineAndSplitIntoTokens(string line)
{
    vector<string>   result;
 
    stringstream lineStream(line);
    string  cell;

    while(getline(lineStream,cell, ','))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if (!lineStream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }
    return result;
}

ostream s();

/**
  Special Logger class for Connections
*/
class ConnectionLogger {
  private:
    ostream *_stream;

  public:
    /**
      Default Connection logger outputs to *cout*
    */
    ConnectionLogger() {
        _stream = &cout;
    }
    /**
      Construct a ConnectionLogger that logs to file
    */
    ConnectionLogger(
      string fileName //!< the name of the file to log to
    ) {
        _stream = new ofstream(fileName);
    }

    void log(
      float value, 
      int cycle
    ) {
         *_stream << cycle << "," << value << endl;      
     }
};

/**
  Abstract Logger Interface
*/
class Logger {
  public:
    /**
      Output a line of text to a log
    */
    virtual void log(string text) = 0;
};

/**
  Concrete Logger that log to a file.
*/
class FileLogger : public Logger {
  private:
   ofstream file;
   function <float (float)>&f;
  public:
      FileLogger(string filename,  function <float (float)> f) : f(f){
         file = ofstream(filename);
      };
     void log(string text) {
         file << now() << ": " << text << endl;      
     }
};

/**
  Concrete Logger that does nothing
*/
class NullLogger : public  Logger {
  void log(string text) {
     }
};

/**
  A NullLLogger Singleton
*/
NullLogger* theNullLogger = new NullLogger();


//Calcuation Functors
/**
  A Functor style class that takes one float and transforms it to another float
*/
class F1 {
  public:
    virtual float calc(const float v1) = 0;
};

/**
  A Functor style class that takes two float value and transforms it to a single float
*/
class F2 {
  public:
    virtual float calc(const float v1, const float v2) = 0;
};
/**
  A Functor style class that takes three float value and transforms it to a single float
*/
class F3 {
  public:
    virtual float calc(const float v1, const float v2, const float v3) = 0;
};


/**
  An abstract class (interface) that allows a name
*/
class Named {
  public:
    virtual string name() = 0;
};

/**
  Abstract interface that defines a port that can accept float values
*/
class In : public Named{
  public:
    virtual void put(float value) = 0;
};

/**
  Abstract interface that defines a port that can produce a float values until an end of file (stream) is detected
*/
class Out : public Named{
  public:
    virtual float get() = 0;
    virtual bool isEOF() = 0;
};

/**
 An In Port for an XForms or Sinks  that have more than one Input
*/
class InPort : public In {
  friend class XForm1;
  friend class XForm2;
  friend class XForm3;
  friend class Sink3;

  private:
    string _name;
    float _value;

  public:
    /**
      Construct an InPort with a given name 
    */
    InPort(
      string name
    ){
      _name = name; //!< the name of the InPort
    }

    /**
      Put a new value in the InPort
    */
    void put(float value) {
       _value = value;
    }

    /**
      return the name of the InPort
    */
    string name() {
       return _name;
    }
};
//Sources


/**
  A Concrete Out port that produces a float value from a given column of a csv structured file
*/
class CsvSource : public Out {
  
  private:
    string _name;
    ifstream *_file;
    int _colNo;
    int _colCount;
    string _line = "";

  public:
    /**
      Construct a CSV Source
      */
    CsvSource(
      string name, //!< a name given to the source, for reporting purposes
      string fileName, //<! the name of the csv file
      int colNo, // <! the column in which the value will be found (zero based)
      int colCount //<! the number of columns expected in the csv file (in fact no used)
    ) {
      _name = name;
      _file = new ifstream(fileName);
      _colNo = colNo;
      _colCount = colCount;
      nextLine();
     }   

    /**
      Get the next value
    */
    float get() {      
      vector<string> values = getNextLineAndSplitIntoTokens(_line);
      nextLine();
      return stof(values[_colNo]);
    };

    /**
      Test for EOF
    */
    bool isEOF() {
      return _file->eof();
    };

    string name() {
      return _name;
    }

    private:
      void nextLine() {
      if(!_file->eof()) {
          getline(*_file,_line);
          if (_line.empty()) {
            nextLine(); // recursion
          }
      } 
    }
};

//Sinks

class CoutSink : public In {
  private:
    string _name;
  public:
    CoutSink(string name) {
      _name = name;
    }
     void put(float value) {
        cout << _name << " recevied " << value << endl;
      };  
    string name() {
      return _name;
    };
};

class CsvSink : public In {
 private:
    ofstream _file;
    string _name;
    int _cycle = 0;
  public:
      CsvSink(string filename) {
         _file = ofstream(filename);
        _name = "File Sink to: " + filename;
      };

      void put(float value) {
        _file << _cycle++ << ", " << value << endl;
      };

      string name() {
        return _name;
      }  
};

class XForm : public Out {
  private:
    string _name;

  protected:
    XForm(string name) {
      _name = name;
    };

  public:
    string name() {
      return _name;
    };

    bool isEOF() {
      return false;
    };
 };

/**
 Class that transforms a single input to a single output 
**/
class XForm1 : public XForm {
  private:
    F1 *_calc;  
    float _value;
  
  public:
    InPort *_v1;
    XForm1(
      string name,
      F1 *calc
    ) : XForm(name) {
      _calc = calc;
      _v1 = new InPort(name + " Port 1");
    };

    float get() {
      return _calc->calc(_v1->_value);
    };
};

/** 
  class that transforms a two inputs into a sinlge value
*/

class XForm2 : public XForm {
  private:
    F2 *_calc;

  public:
    InPort *_v1;
    InPort *_v2;

    XForm2(
      string name,
      F2 *calc
    ) : XForm(name) {
       _calc = calc;
      _v1 = new InPort(name + " Port 1");
      _v2 = new InPort(name + " Port 2");
    };
 
    float get() {
      return _calc->calc(_v1->_value, _v2->_value);
    }; 
};

class XForm3 : public XForm {
  private:
    F3 *_calc;

  public:
    InPort *_v1;
    InPort *_v2;
    InPort *_v3;

    XForm3(
      string name,
      F3 *calc
    ) : XForm(name) {
      _calc = calc;
      _v1 = new InPort(name + " Port 1");
      _v2 = new InPort(name + " Port 2");
      _v3 = new InPort(name + " Port 3");
    };
 
    float get() {
      return _calc->calc(_v1->_value, _v2->_value, _v3->_value);
    }
};

/**
 Class that transforms a single input and the previous inout to a single output 
**/
class XForm1WithLastValue: public In, public Out {
  private:
    string _name;
    F2 *_calc;
    float _value;
    float _prevValue;

  public:
    XForm1WithLastValue(
      string name,
      float firstValue, 
      F2 *calc 
    ) {
      _name = name,
      _calc = calc;
      _prevValue = firstValue;
    };

    float get() {
      return _value;
    };

    void put(float value) {     
      _value = _calc->calc(_prevValue, value);
      _prevValue = value;     
   };

    string name() {
      return _name;
    };

   bool isEOF() {
      return false;
    };
};

class Connector {
  private:
    Out *_out;
    vector<In*> _inPorts;
    ConnectionLogger *_logger;
  public:

    Connector(
      Out *out,
      In*  inPort,
      ConnectionLogger *logger
    ){
      _out = out;
      _logger = logger;
      _inPorts = {inPort};
    };

    Connector(
      Out *out,
      In*  inPort1,
      In*  inPort2,
      ConnectionLogger *logger
    ){
      _out = out;
      _logger = logger;
      _inPorts = {inPort1, inPort2};
    };

    bool run(int cycleNo) {
      if (!_out->isEOF()) {
        float value = _out->get();
        _logger->log(value, cycleNo);
        for (int i = 0; i < _inPorts.size(); i++ ) {
          _inPorts[i]->put(value);
        }
        return true;
      } else {
        return false;
      }
    };

};



// == start of app specific stuff
/**
  Tranforms a sinlge input into an output by doubling the value
*/
class Doubler : public F1 {
    float calc(const float value) {
      return value * 2;
    }
};


class Sensor1Conversion : public F1 {
  public: 
    float calc(const float value) {
      return 2.0/3.0 * sqrt(value);
    }
};

class Sensor2Conversion : public F2 {
  public:
    float calc(float v1, float v2) {
      return v2 - v1;
    }
};

class Scale : public F1 {
  private:
    float _alpha;
    float _beta;

  public:
    Scale(
      float alpha,
      float beta    
    ) {
      _alpha = alpha;
      _beta = beta;
    }

    float calc(float  v1) {
      return _alpha * (v1 - _beta);    
    }
};

class SensorFusionCalc : public F3 {
  public:
    float calc(const float v1, const float v2, const float v3) {
      if (v2 == 0.0) {
        return 1;
      };
      return 3.0 * (v1 - v3)/v2 - 3.0;
    }
};

class MotorCalc : public F1 {
  private:
    bool _invert;

  public:
    MotorCalc(bool invert = false) {
      _invert = invert;
    }
    float calc(const float value) {
      float ret = value;
      if (value > 1.0) {
        ret = 1.0;
      } else if (value  <-1.0) {
        ret = -1.0;
      }
      if (_invert) {
        ret = ret * -1;
      }
      return ret;
    };
};

int main() {
   cout << "Program Started" << endl;
  
   //NOdes
  CsvSource *sensor1Source = new CsvSource(
    "Sensor1Source",
    "./_StreamData/sensor_1.csv",
    1,
    2
  );  
  CsvSource *sensor2Source = new CsvSource(
    "Sensor1Source",
    "./_StreamData/sensor_2.csv",
    1,
    2
  ); 
  
  CsvSource *sensor3Source = new CsvSource(
    "Sensor1Source",
    "./_StreamData/sensor_3.csv",
    1,
    2
  );
  XForm1 *conversion1 = new XForm1("Conversion1", new Sensor1Conversion());
  XForm1WithLastValue *conversion2 = new XForm1WithLastValue(
    "Conversion2",
    -1,  
    new Sensor2Conversion() 
  );
  XForm1 *scale1 = new XForm1("Scale1", new Scale(2.7, 1.0));
  XForm1 *scale2 = new XForm1("Scale2", new Scale(0.7, -0.5));
  XForm1 *scale3 = new XForm1("Scale3", new Scale(1.0, 0.2));
  XForm3 *sensorFusion = new XForm3("SensorFusion", new SensorFusionCalc());
  XForm1 *motorA = new XForm1("MotorA", new MotorCalc());
  XForm1 *motorB = new XForm1("MotorB", new MotorCalc(true));
  CsvSink *motorASink = new CsvSink("motorA.cvs");
  CsvSink *motorBSink = new CsvSink("motorB.cvs");

//  CoutSink *debugSink = new CoutSink("debug");
  
  //Connectors
  Connector *c1 = new Connector(
    sensor1Source,
    conversion1->_v1,
    new ConnectionLogger("./c1.txt")
  );
  Connector *c2 = new Connector(
    sensor2Source,
    conversion2,
    new ConnectionLogger("./c2.txt")
  );
  Connector *c3 = new Connector(
    conversion1,
    scale1->_v1,
    new ConnectionLogger("./lp_A.cvs")
  );
  Connector *c4 = new Connector(
    conversion2,
    scale2->_v1,
    new ConnectionLogger("./lp_B.cvs")
  );
  Connector *c5 = new Connector(
    sensor3Source,
    scale3->_v1,
    new ConnectionLogger("./lp_c5.cvs")
  );
  Connector *c6 = new Connector(
    scale1,
    sensorFusion->_v1,
    new ConnectionLogger("./lp_C.cvs")
  );
  Connector *c7 = new Connector(
    scale2,
    sensorFusion->_v2,
    new ConnectionLogger("./lp_D.cvs")
  );
  Connector *c8 = new Connector(
    scale3,
    sensorFusion->_v3,
    new ConnectionLogger("./lp_E.cvs")
  );
  Connector *c9 = new Connector(
    sensorFusion,
    motorA->_v1, 
    motorB->_v1,
    new ConnectionLogger("./lp_F.cvs")
  );
  Connector *c10 = new Connector(
    motorA,
    motorASink,
    new ConnectionLogger("./c10.txt")
  );
  Connector *c11 = new Connector(
    motorB,
    motorBSink,
    new ConnectionLogger("./c11.txt")
  );


  vector<Connector*> allConns = {c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11};
  //vector<Connector*> allConns = { c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11};
  bool done = false;
  int count = 0;
  while(!done) {
    for (int i = 0; i < allConns.size(); i++) {
      if (!allConns[i]->run(count)) {
        done = true;
        break;
      };
    };
    count++;
    //if (count > 1) break;
  };
  cout << "Program Ended" << endl;
};

enum SensorType { SENSOR_1 = 1, SENSOR_2 = 2, SENSOR_3 = 3};

