#include"Person.h"

/* 0: Persona sana
* 1: Persona infectada
* 2: Persona inmune
* 3: Persona muerta*/
Person::Person() {
	this->state = 0; //Personas nacen sanas
	this->sick_time = 0; //Personas nacen con 0 tics enfermas
}

void Person::setState(int state) {
	this->state = state;
}

int Person::getX() {
	return x;
}

int Person::getY() {
	return y;
}

void Person::setY(int y) {
	this->y = y;
}

void Person::setX(int x) {
	this->x = x;
}

int Person::getState() {
	return state;
}

int Person::getSick() {
	return sick_time;
}

void Person::setSick(int sick_time) {
	this->sick_time = sick_time;
}