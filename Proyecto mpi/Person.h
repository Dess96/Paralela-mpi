#pragma once
#ifndef Persona
#define Persona

class Person {
private:
	int x; //Posicion en x
	int y; //Posicion en y
	int state; //Estado
	int sick_time; //Tiempo que lleva la persona enferma
public:
	Person();
	void setX(int);
	int getX();
	void setY(int);
	int getY();
	void setState(int);
	int getState();
	void setSick(int);
	int getSick();
};
#endif // !Persona