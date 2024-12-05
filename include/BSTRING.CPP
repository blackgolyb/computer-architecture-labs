#include <iostream.h>

#include <iomanip.h>

//#include "bstring.h"

#include <string.h>

class CString
{
	private:

	int str_size;
	char *str;

	public:

	// ???????????
	CString (): str_size (0)
	{
		set ("");
	}
	CString (const char *s)
	{
		str_size = strlen (s);
		set (s);
	}
	CString (const CString &s)
	{
		str_size = s.str_size;
		set (s.str);
	}

	// ??????????
	~CString ()
	{
		delete [] str;
	}

	// ????????? ?-??
	int size () const;
	void size (int);

	char *get () const;
	void set (const char *);
	void clear ();
	};

// ?-??
int CString::size () const
{
	return str_size;
}

void CString::size (int s)
{
	str_size = s;
}

char *CString::get () const
{
	return str;
}

void CString::set (const char *s)
{
	int good = 1;
	for (int i = 0; i < str_size; i ++)
	{
		if (s[i] != '0' && s[i] != '1')
		{
			good = 0;
                        break;
                }
	}
	if (good)
        {
		str = new char[str_size + 1];
		strcpy (str, s);
	}
	else
	{
		str = new char[1];
		strcpy (str, "0");
        }
}

void CString::clear ()
{
	str_size = 0;
	set ("");
}


//#include "cstring.h"

class BString: public CString
{
	private:

	// ?-??
	void scale (char *, char *);

	public:

	// ???????????
	BString (): CString () {};
	BString (const char *s)
	{
		CString::size (strlen (s));
		CString::set (s);
	}
	BString (BString &s)
	{
		CString::size (s.size ());
		CString::set (s.get ());
	}

	// ??????????
	~BString () {}

	// =
	BString &operator = (BString &);
	// ?-??
	// ???????????? =
	BString &operator = (char *);
	// ??????? -
	BString operator - () const;
	// ???????? +
	BString operator + (const BString &);
	// ????????? ==
	int operator == (const BString &);
	};

// ?-??
void BString::scale (char *s1, char *s2)
{
	int size1 = strlen (s1);
	int size2 = strlen (s2);

	if (size1 > size2)
	{
		char *b = new char[size1 + 1];
		int n = size1 - size2;

		strcpy (b, s2);

		int i;
		for (i = 0; i < n; i ++) s2[i] = '0';
		for (i = n; i < size1 + 1; i ++) s2[i] = b[i - n];

		delete [] b;
	}
	else if (size2 > size1)
	{
		char *b = new char[size2 + 1];
		int n = size2 - size1;

		strcpy (b, s1);

		int i;
		for (i = 0; i < n; i ++) s1[i] = '0';
		for (i = n; i < size2 + 1; i ++) s1[i] = b[i - n];

		delete [] b;
	}
}

BString &BString::operator = (BString &s)
{
	CString::size (s.size ());
	CString::set (s.get ());

	return *this;
}

// =
BString &BString::operator = (char *s)
{
	CString::size (strlen (s));
	CString::set (s);

	return (*this);
}

// -
BString BString::operator - () const
{
	int size = CString::size ();

	char *b = new char[size + 1];

	strcpy (b, CString::get ());

	for (int i = 0; i < size; i ++)
	{
		if (b[i] == '0') b[i] = '1';
		else b[i] = '0';
	}

	BString r (b);
	BString tmp ("1");
	r = r + tmp;

	delete [] b;

	return (r);
}

// +
BString BString::operator + (const BString &s)
{
	int size1 = s.size ();
	int size2 = CString::size ();
	int size = (size1 > size2) ? size1 : size2;

	char *b1 = new char[size1 + 1];
	char *b2 = new char[size2 + 1];
	char *b = new char[size + 1];

	strcpy (b1, s.get ());
	strcpy (b2, CString::get ());


	scale (b1, b2);

	int m = 0;
	b[0] = '0';
	for (int i = size; i >= 0; i --)
	{
		if (b1[i] == '0' && b2[i] == '0')
		{
			b[i] = '0';
			if (m)
			{
				b[i] = '1';
				m = 0;
			}
		}
		else if ((b1[i] == '1' && b2[i] == '0') || (b1[i] == '0' && b2[i] == '1'))
		{
			b[i] = '1';
			if (m)
			{
				b[i] = '0';
				m = 1;
			}
		}
		else if (b1[i] == '1' && b2[i] == '1')
		{
			b[i] = '0';
			m = 1;
		}
		else
		{
			b[i] = '0';
		}
	}
	if (m) {/*????????????*/}

	b[size] = '\0';


	BString r (b);

	delete [] b1;
	delete [] b2;
	delete [] b;

	return (r);
}

// ==
int BString::operator == (const BString &s)
{
	int size1 = s.size ();
	int size2 = CString::size ();
	int size = size1 > size2 ? size1 : size2;

	char *b1 = new char[size1 + 1];
	char *b2 = new char[size2 + 1];

	strcpy (b1, s.get ());
	strcpy (b2, CString::get ());

	scale (b1, b2);

	int m = 1;
	for (int i = size - 1; i >= 0; i --)
	{
		if (b1[i] != b2[i])
		{
			m = 0;
			break;
		}
	}

	return (m) ? 1 : 0;
}


//using namespace std;

int main ()
{
	const int SIZE = 3;
	const char ESC = 27;

	BString *a = new BString[SIZE];

	char ch;
	int i = 0, j = 0;

	cout << "1 - Fill   2 - Show   3 - Sum   4 - Compare   5 - Otric   ESC - Exit" << endl;

	do
	{
		switch (ch)
		{
			// заполняем массив
			case '1':
				for (i = 0; i < SIZE; i ++)
				{
					char input[100];

					cout << "Enter [" << i << "]: ";
					cin >> setw (100) >> input;

					a[i] = input;
				}
				break;
			// показываем все элементы массива
			case '2':
				for (i = 0; i < SIZE; i ++) cout << a[i].get () << endl;
				break;
			// складываем
			case '3':
			// сравниваем
			case '4':
				do
				{
					cout << "Enter [first]: ";
					cin >> i;
				} while (i >= SIZE);

				do
				{
					cout << "Enter [second]: ";
					cin >> j;
				} while (j >= SIZE);

				if (ch == '3')
				{
					BString res ("");
					res = a[i] + a[j];

					cout << a[i].get () << " + " << a[j].get () << " = " << res.get () << endl;
				}
				else if (ch == '4')
				{
					if (a[i] == a[j])
					cout << a[i].get () << " == " << a[j].get () << endl;
					else
					cout << a[i].get () << " != " << a[j].get () << endl;
				}
				break;
			// дополнение до двух
			case '5':
				do
				{
					cout << "Enter [i]: ";
					cin >> i;
				} while (i >= SIZE);

				cout << a[i].get () << " otric = ";
				a[i] = -a[i];
				cout << a[i].get () << endl;

				break;
		}
		cout << "Enter num: ";
		cin >> ch;
	} while (ch != ESC);

	return (0);
}
