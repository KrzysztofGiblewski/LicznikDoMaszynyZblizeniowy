#include <LiquidCrystal_I2C.h> 
#include <Wire.h>
long ilePacz = 0;   //ile sztuk w bierzacej paczce
long ileKart = 0;   //ile pelnych kartonow
long ileWszy = 0;   //ile wszystkich produktow
long ustawPacz = 100; // zadaje ilosc w paczce np 100szt
long ustawKart = 0; //zadaje ilosc w kartonie np 1000sz
long ileWOsta = 0; //ile w ostatnim nie pelnym kartonie
int ekrany = 0;
int poIle = 1; //po ile dodawać kolejne sztuki jeśli robi dwa na raz to niech dodaje po 2 sztuki
int coIle = 1;
int takty = 0;
int sztuka = 1;
int wartoscImpulsu = 0;
int wartoscZblizImp=0;
int popWartoscImpu  = 0;
char impuls = 1; //wartosc 0 lub 1 zeby po podaniu ciaglego napiecia nie naliczal kolejnych sztuk
double napImpulsu = 4.0; //minimalna wartość impulsu w voltach dla impulsu
double zeroNapiecia = 0.02; // wartosc napiecia ponirzej ktorego uznajemy za zanik impulsu
int opuznij = 10; //przerwa miedzy cyklami
int sygnal = 1; // wyprzedzenie przed iloma workami ma piszcec
int dlugoscSygnal = 0; //zabezpieczenie przed zatrzymaniem na piszczacym worku
int wyjdzZMenu = 0;
int ekranyUstawien = 0; //czy rozszerzone menu czy krutkie
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27         A4 SDA        A5 SCL
  
void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("0");
  pinMode(A0, INPUT_PULLUP); //przycisk dodawania sztuki A0
  pinMode(A1, INPUT_PULLUP); // przycisk odejmowania A1
  pinMode(A2, INPUT_PULLUP); //przycisk wyboru A2
  pinMode(A3, OUTPUT); //Konfiguracja A3 jako wyjście dla buzzera
  //modul na pinie A4 SDA  dla I2C
  //    i A5 SCL dla I2C
  wartoscImpulsu = analogRead(A6); // pin A6 czyta wartosc napiecia inpulsu
  wartoscZblizImp=analogRead(A7); //pin A7 z modulu zblizeniowego
  
  
}

void loop() {

  wartoscImpulsu = analogRead(A6); //zczytuje impuls z licznika maszyny A6
  wartoscZblizImp = analogRead(A7); //zczytuje impuls z modulu zblizeniowego A7
  
  delay(opuznij); //daje małe opuźnienie żeby impuls był pojedyńczy
  
  
  if (dlugoscSygnal < 50)
    dlugoscSygnal++;

  if (wartoscZblizImp < zeroNapiecia){ //jak napiecie zaniknie to mozna znowu liczyc impuls
    impuls = 1;
    Serial.println(wartoscZblizImp* (5.0 / 1024.0));
  }
  if ((wartoscZblizImp * (5.0 / 1024.0) > napImpulsu) && impuls == 1 ) { //warunek minimalnego napiecia dla impulsu zeby dodac 1 impuls musi byc poprzedni zero
    dodaj(sztuka * poIle);
    impuls = 0;
    dlugoscSygnal = 0;
    
  Serial.println(wartoscZblizImp* (5.0 / 1024.0));
  
  }
 
  if (digitalRead(A2) == LOW)   { //przycisk wyboru A2
    zmienEkrany();
    wyjdzZMenu = 0;

  }

  if (ekrany > 0) //jesli na innym ekranie menu to po czasie wyjdz do ekranu pierwszego
    if (wyjdzZMenu < 305)
      wyjdzZMenu++;

  liczKart(); //licze kartony
  liczPacz(); //licze zeby było tyle ile ma mieć paczka
  wyswietl();
  buzerr();
}
void wyswietl() {
  pierwszaLinia();
  switch (ekrany)
  {
    case 0:             // bierzacza ilosc w paczce wlasnie robionej
      {
        if (digitalRead(A0) == LOW)   {
          dodaj(sztuka * poIle);
        }
        if (digitalRead(A1) == LOW)   {
          odejmij(sztuka * poIle);
        }
        drugaLinia("", ilePacz, " sztuk w paczce           ", 0);
        break;
      }
    case 1:                     // bierzaca ilosc sztuk w kartonie
      {
        if (digitalRead(A0) == LOW)   {
          dodaj(sztuka * poIle);
        }
        if (digitalRead(A1) == LOW)   {
          odejmij(sztuka * poIle);
        }
        drugaLinia("ost karon ", ileWOsta, "szt    ", ileKart);
        break;
      }
    case 2:                             //ustaw ile w paczce
      {
        if (digitalRead(A0) == LOW)   {
          ustawPacz += 5;
          delay(250);
        }
        if (digitalRead(A1) == LOW)   {
          if (ustawPacz > 4)
            ustawPacz -= 5;
          delay(250);
        }
        drugaLinia("PACZKA to ", ustawPacz, " szt      ", 0);
        break;
      }
    case 3:                     // ilosc kartonow
      {
        if (digitalRead(A0) == LOW)   {
          dodaj(sztuka * poIle);
        }
        if (digitalRead(A1) == LOW)   {
          odejmij(sztuka * poIle);
        }
        drugaLinia("", ileKart, " pelne kartony      ", 0);
        break;
      }

    case 4:                             //ustaw ile w kartonie
      {
        if (digitalRead(A0) == LOW)   {
          ustawKart += ustawPacz;
          delay(250);
        }
        if (digitalRead(A1) == LOW)   {
          if (ustawKart >= ustawPacz)
            ustawKart -= ustawPacz;
          delay(250);
        }
        drugaLinia("KARTON to ", ustawKart, " szt           ", 0);
        break;
      }
    case 5:                             //Zeruj liczniki
      {
        if (digitalRead(A0) == LOW)   {     //jak wcisne + to wychodzimy ekran wyrzej
          ekrany = -1;
          zmienEkrany();
        }
        if (digitalRead(A1) == LOW)   {     //jak wcisne - to kasuje liczniki
          ileWszy = 0;
          ilePacz = 0;
          ekrany = -1;
          zmienEkrany();
        }
        drugaLinia("+wyjdz+ -skasuj- ", 0, "", 0);
        break;
      }
    case 6: //Ustawienia tak nie
      {
        if (digitalRead(A0) == LOW)   { //jak tak to 1
          ekranyUstawien = 1;
          delay(250);
        }
        if (digitalRead(A1) == LOW)   { //jak nie to 0
          ekranyUstawien = 0;
          delay(250);
        }
        lcd.setCursor(0, 1);
        lcd.print("USTAWIENIA ");
        if (ekranyUstawien==1)
        lcd.print("+ TAK ");
        if(ekranyUstawien==0)
        lcd.print("- NIE ");
        break;
      }

    case 7:                             //ustaw po ile ma sumowac 1 czy np 2 jak na dwa tory
      {
        if (digitalRead(A0) == LOW)   {
          poIle++;
          delay(250);
        }
        if (digitalRead(A1) == LOW)   {
          if (poIle > 0)
            poIle--;
          delay(250);
        }
        drugaLinia("LICZ PO ", poIle, " szt             ", 0);
        break;
      }
    case 8:                             //ustaw co ile ma dodawac 1 sztuke np co 2 uderzenia
      {
        if (digitalRead(A0) == LOW)   {
          coIle++;
          delay(200);
        }
        if (digitalRead(A1) == LOW)   {
          if (coIle > 1)
            coIle--;
          delay(200);
        }
        drugaLinia("LICZ ", coIle, " takt jak", 1);
        break;
      }

    case 9:                             //ustaw delay miedzy impulsami
      {
        if (digitalRead(A0) == LOW)   {
          opuznij += 1;
          delay(200);
        }
        if (opuznij > 1)
          if (digitalRead(A1) == LOW)   {
            opuznij -= 1;
            delay(200);
          }
        drugaLinia("DELAY ", opuznij, " takt    ", 0);
        break;
      }
    case 10:                             //ustaw napiecie wejsciowe impulsu
      {
        if (digitalRead(A0) == LOW)   {
          napImpulsu += 0.02;
          delay(50);
        }
        if (digitalRead(A1) == LOW)   {
          if (napImpulsu > 0)
            napImpulsu -= 0.02;
          delay(50);
        }
        lcd.setCursor(0, 1);
        lcd.print("MIN V IMP ");
        lcd.print(napImpulsu);
        lcd.print("V  ");
        break;
      }

    case 11:                             // tu ustawiam napiecie ponizej ktorego traktujemy jak zero
      {
        if (digitalRead(A0) == LOW)   {
          zeroNapiecia += 0.01;
          delay(100);
        }
        if (digitalRead(A1) == LOW)   {
          if (zeroNapiecia >= 0.01)
            zeroNapiecia -= 0.01;
          delay(100);
        }
        
        lcd.setCursor(0, 1);
        lcd.print("MAX V ZER ");
        lcd.print(zeroNapiecia);
        lcd.print("V");
        break;
      }
    case 12:                             // tu ustawiam wyprzedzenie sygnalu
      {
        if (digitalRead(A0) == LOW)   {
          sygnal += 1;
          delay(250);
        }
        if (digitalRead(A1) == LOW)   {
          if (sygnal > 0)
            sygnal -= 1 ;
          delay(250);
        }
        lcd.setCursor(0, 1);
        lcd.print("SYGNAL ");
        lcd.print(sygnal);
        if (sygnal > 0)
          lcd.print(" PRZED  ");
        if (sygnal == 0)
          lcd.print(" WYLACZONY");
        break;
      }


  }

}
void dodaj(int ile) {
  takty++;
  if (takty == coIle || takty > coIle) {
    ileWszy += ile;
    takty = 0;
  }
  ilePacz = ileWszy % ustawPacz;
  if (wyjdzZMenu > 300) //jak dlugo nie dotykane meni to wyjdz do ekranu pierwszego
    ekrany = 0;


  delay(100);
}
void odejmij(int ile) {
  if (ileWszy > 0) {
    ileWszy -= ile;
    ilePacz = ileWszy % ustawPacz;

  }
  delay(100);
}
void liczPacz() {
  if (ilePacz >= ustawPacz)
    ilePacz = 1;
}
void liczKart() {
  if (ustawKart > 0) {
    ileKart = ileWszy / ustawKart;
    ileWOsta = ileWszy % ustawKart;
  } else {
    ileKart = 0 ;
    ileWOsta = ileWszy;
  }

}
void zmienEkrany() {
  delay(200);
  ekrany++;
    lcd.begin(16, 2);
  if (ekranyUstawien == 1)
    if (ekrany > 12)
      ekrany = 0;
      if(ekranyUstawien==0)
  if (ekrany > 6)
    ekrany = 0;
}
void pierwszaLinia() {
  lcd.setCursor(0, 0);
  lcd.print(ileWszy);
  //  lcd.print("  ");
  //  lcd.print(dlugoscSygnal);
  //  lcd.print("  ");
  //  lcd.print(wyjdzZMenu);
  lcd.print(" szt wszystkich ");
}
void drugaLinia(String raz, long dwa, String trzy, long cztery) {
  lcd.setCursor(0, 1);
  lcd.print(raz);
  lcd.print(dwa);
  lcd.print(trzy);
  lcd.print(cztery);

}
void buzerr() {

  if (sygnal > 0) {
    if (sygnal < poIle)
      sygnal = poIle;
    if ((ilePacz >= ( ustawPacz - sygnal )) && (dlugoscSygnal < 50)) {
      digitalWrite(A3, HIGH);

    }
  }
  if (ilePacz >= 0 && ilePacz <= poIle || dlugoscSygnal >= 50) {
    digitalWrite(A3, LOW);
  }

}
