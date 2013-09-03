// File:  gpio.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  C++ wrapper for Wiring Pi general-purpose I/O function calls.

#ifndef GPIO_H_
#define GPIO_H_

class GPIO
{
public:
	enum DataDirection
	{
		DirectionInput,
		DirectionOutput,
		DirectionPWMOutput
	};

	enum PullResistance
	{
		PullOff,
		PullUp,
		PullDown
	};

	GPIO(int pin, DataDirection direction = DirectionInput);

	void SetDataDirection(DataDirection direction);
	void SetPullUpDown(PullResistance state);
	void SetOutput(bool high);
	bool GetInput(void);

protected:
	const int pin;

private:
	DataDirection direction;
};


#endif// GPIO_H_
