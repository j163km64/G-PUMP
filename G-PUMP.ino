#define RANGESTEP 32000
#define VOLUME 500

#define PULSE_PIN 9
#define DIR_PIN 8
#define ENABLE_PIN 10
#define STOP_SENSOR A0
#define SENSOR_THRESHOLD 200

#define ON 1
#define OFF 0

//#define DEFAULT_SPEED 100

#include <SimpleCLI.h> // Include Library
//#include <FreeRTOS.h>

// Create CLI Object
SimpleCLI cli;

// Commands
Command inject;
Command stop;
Command pulse;
Command fill;
Command constantFlow;
Command pulseFlow;

Command default_speed;

// Global Variables
int PUMP_VOLUME = 0;
bool PUMP_DIR = 0;
int DEFAULT_SPEED = 100; // unit: ul/s

// Global Function

inline int VOLUME2STEP(int volume)
{
    return volume * RANGESTEP / VOLUME; // return STEP number
}

inline int SPEED_DELAY_CONVERSION(int SPEED = DEFAULT_SPEED)
{
    return VOLUME * 500000 / (RANGESTEP * SPEED); // return Delay in microsecond
}

/*------Pump Control------*/
// Pump Fill
void pumpFill()
{
    digitalWrite(ENABLE_PIN, ON);
    delay(10); // wait for the pump to be ready (10ms)
    digitalWrite(DIR_PIN, LOW);//Move to 0 Volume

    while (1)
    {
        digitalWrite(PULSE_PIN, HIGH);
        delayMicroseconds(SPEED_DELAY_CONVERSION());
        digitalWrite(PULSE_PIN, LOW);
        delayMicroseconds(SPEED_DELAY_CONVERSION());
        if (analogRead(STOP_SENSOR) > SENSOR_THRESHOLD)
        {
            break;
        }
    }

    Serial.println("Pump has been emptied, ready for fill in 5 seconds");
    for (int i = 3; i > 0; i--)
    {
        Serial.println(i);
        delay(1000);
    }

    // Start Fill
    digitalWrite(DIR_PIN, HIGH);

    for (int i = 0; i < RANGESTEP; i++)
    {
        digitalWrite(PULSE_PIN, HIGH);
        delayMicroseconds(SPEED_DELAY_CONVERSION);
        digitalWrite(PULSE_PIN, LOW);
        delayMicroseconds(SPEED_DELAY_CONVERSION);
    }
    Serial.println("Pump has been filled");
    digitalWrite(ENABLE_PIN, OFF);
}

void pump_inject(int volume)
{
    digitalWrite(ENABLE_PIN, ON);
    delay(10); // wait for the pump to be ready (10ms)
    digitalWrite(DIR_PIN, LOW);
    for (int i = 0; i < VOLUME2STEP(volume) - 1; i++)
    {
        digitalWrite(PULSE_PIN, HIGH);
        delayMicroseconds(SPEED_DELAY_CONVERSION());
        digitalWrite(PULSE_PIN, LOW);
        delayMicroseconds(SPEED_DELAY_CONVERSION());
    }

    Serial.println("Constant Flow has been completed");
    digitalWrite(ENABLE_PIN, OFF);
}

void constflow(int volume, int speed)
{
    digitalWrite(ENABLE_PIN, ON);
    delay(10); // wait for the pump to be ready (10ms)
    digitalWrite(DIR_PIN, LOW);
    Serial.println("Constant Flow has been started");
    for (int i = 0; i < VOLUME2STEP(volume) - 1; i++)
    {
        digitalWrite(PULSE_PIN, HIGH);
        delayMicroseconds(SPEED_DELAY_CONVERSION(speed));
        digitalWrite(PULSE_PIN, LOW);
        delayMicroseconds(SPEED_DELAY_CONVERSION(speed));
    }

    Serial.println("Constant Flow has been completed");
    digitalWrite(ENABLE_PIN, OFF);
}

void pulseflow(int volume, int width, int speed, int interval)
{
    digitalWrite(ENABLE_PIN, ON);
    delay(10); // wait for the pump to be ready (10ms)
    digitalWrite(DIR_PIN, LOW);
    for (int i = 0; i < volume / width - 1 ; i++)
    {
        for (int i = 0; i < VOLUME2STEP(width) - 1; i++)
        {
            digitalWrite(PULSE_PIN, HIGH);
            delayMicroseconds(SPEED_DELAY_CONVERSION(speed));
            digitalWrite(PULSE_PIN, LOW);
            delayMicroseconds(SPEED_DELAY_CONVERSION(speed));
        }
        delay(interval);
    }



    Serial.println("Pulse Flow has been completed");
    digitalWrite(ENABLE_PIN, OFF);
}

// Callback function for command
void injectCallback(cmd *c)
{
    Command cmd(c); // Create wrapper object

    int argNum = cmd.countArgs(); // Get number of arguments
    int sum = 0;
    if (argNum == 0 | argNum > 1)
    {
        Serial.println("Wrong Argument Count. Usage: inject <volume>");
        return;
    }
    String argValue = cmd.getArg(0).getValue();
    int argIntValue = argValue.toInt();

    if (argIntValue > 0)
    {
        Serial.print("Injecting ");
        Serial.print(argIntValue);
        Serial.println("ul");

        pump_inject(argIntValue);
    }
}

void constFlowCallback(cmd *c)
{
    Command cmd(c); // Create wrapper object

    int argNum = cmd.countArgs(); // Get number of arguments
    int sum = 0;
    if (argNum == 0 | argNum > 2)
    {
        Serial.println("Wrong Argument Count. Usage: constFlow <volume> <speed>");
        return;
    }
    String argVolume = cmd.getArg("volume").getValue();
    String argSpeed = cmd.getArg("speed").getValue();
    int argIntVolume = argVolume.toInt();
    int argIntSpeed = argSpeed.toInt();

    if (argIntVolume > 0 && argIntSpeed > 0)
    {
        Serial.print("Constant Flow ");
        Serial.print(argIntVolume);
        Serial.print(" ul at ");
        Serial.print(argIntSpeed);
        Serial.println(" ul/s");

        constflow(argIntVolume, argIntSpeed);
    }
}

void pulseFlowCallback(cmd *c)
{
    Command cmd(c); // Create wrapper object

    int argNum = cmd.countArgs(); // Get number of arguments
    int sum = 0;
    if (argNum == 0 | argNum > 4)
    {
        Serial.println("Wrong Argument Count. Usage: pulseFlow <volume> <width> <speed> <interval>");
        return;
    }
    String argVolume = cmd.getArg("volume").getValue();
    String argWidth = cmd.getArg("width").getValue();
    String argSpeed = cmd.getArg("speed").getValue();
    String argInterval = cmd.getArg("interval").getValue();
    int argIntVolume = argVolume.toInt();
    int argIntWidth = argWidth.toInt();
    int argIntSpeed = argSpeed.toInt();
    int argIntInterval = argInterval.toInt();

    if (argIntVolume > 0 && argIntWidth > 0 && argIntSpeed > 0 && argIntInterval > 0)
    {
        Serial.print("Pulse Flow ");
        Serial.print(argIntVolume);
        Serial.print(" ul per ");
        Serial.print(argIntWidth);
        Serial.print(" ul at ");
        Serial.print(argIntSpeed);
        Serial.print(" ul/s with ");
        Serial.print(argIntInterval);
        Serial.println("ms interval");

        pulseflow(argIntVolume, argIntWidth, argIntSpeed, argIntInterval);
    }
}

void fillCallback(cmd *c)
{
    Command cmd(c); // Create wrapper object
    pumpFill();
}

void speed(cmd *c){
    Command cmd(c); // Create wrapper object

    int argNum = cmd.countArgs(); // Get number of arguments
    int sum = 0;
    if (argNum == 0 | argNum > 1)
    {
        Serial.println("Wrong Argument Count. Usage: inject <volume>");
        return;
    }
    String argValue = cmd.getArg(0).getValue();
    int argIntValue = argValue.toInt();

    if (argIntValue > 0)
    {
        DEFAULT_SPEED = argIntValue;
        Serial.print("default speed has been set to");
        Serial.print(argIntValue);
        Serial.println(" ul/s");


    }

}

// Callback in case of an error
void errorCallback(cmd_error *e)
{
    CommandError cmdError(e); // Create wrapper object

    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand())
    {
        Serial.print("Did you mean \"");
        Serial.print(cmdError.getCommand().toString());
        Serial.println("\"?");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("FluidStation V1.0");
    Serial.println("Initializing...");

    // pump setup
    pinMode(PULSE_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    pinMode(STOP_SENSOR, INPUT);
    
    digitalWrite(ENABLE_PIN, OFF);

    cli.setOnError(errorCallback); // Set error Callback

    // inject = cli.addCommand("inject", injectCallback); // Bind Callback to inject command

    inject = cli.addBoundlessCommand("inject", injectCallback); // Bind Callback to inject command
    
    constantFlow = cli.addCommand("constFlow", constFlowCallback); // Bind Callback to inject command
    constantFlow.addArgument("volume");                         // Add Argument to ping command
    constantFlow.addArgument("speed");                          // Add Argument to ping command

    pulseFlow = cli.addCommand("pulseFlow", pulseFlowCallback); // Bind Callback to inject command
    pulseFlow.addArgument("volume");                         // Add Argument to ping command
    pulseFlow.addArgument("width");                          // Add Argument to ping command
    pulseFlow.addArgument("speed");                          // Add Argument to ping command
    pulseFlow.addArgument("interval");                       // Add Argument to ping command

    fill = cli.addCommand("fill", fillCallback);
    default_speed = cli.addBoundlessCommand("default_speed", speed);

}

void loop()
{
    // Check if user typed something into the serial monitor
    if (Serial.available())
    {
        // Read out string from the serial monitor
        String input = Serial.readStringUntil('\n');

        Serial.print("# ");
        Serial.println(input);

        // Parse the user input into the CLI
        cli.parse(input);
    }

    if (cli.errored())
    {
        CommandError cmdError = cli.getError();

        Serial.print("ERROR: ");
        Serial.println(cmdError.toString());

        if (cmdError.hasCommand())
        {
            Serial.print("Did you mean \"");
            Serial.print(cmdError.getCommand().toString());
            Serial.println("\"?");
        }
    }
}
