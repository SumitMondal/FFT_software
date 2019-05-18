// Class declaration for Complex number class


#include <iostream>
#include <string>

// C++ now includes a type for complex numbers std::complex which could also be used.

class Complex {
public:
  //Constructors;
  Complex();
  Complex(double r, double i); // Real and imaginary parts
  explicit Complex(double r);           // Real part only, imag = 0.
  // Overloaded operators
  Complex operator+ (const Complex& b) const;
  Complex operator- (const Complex& b) const;
  Complex operator* (const Complex& b) const;
  // Required member functions
  Complex Mag()   const; //Returns the magnitude of the complex number
  Complex Angle() const; //returns the angle of the complex number
  Complex Conj()  const; //returns the complex conjugate of the Complex number
  void    Print() const; // Print the complex value
  // Data Members
public:
  double real;
  double imag;
};

// Global function to output a Complex value
std::ostream& operator << (std::ostream &os, const Complex& c);







