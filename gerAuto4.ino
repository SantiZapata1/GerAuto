//librerias
#include <Bounce2.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

//menues
#define MODO 0

#define SENSORES 1
#define SENSORES_2 11

#define ACTUADORES 2
#define ACTUADORES_2 8
#define ACTUADORES_3 17
#define ACTUADORES_4 18

#define CONFIGURACION 3
#define CONFIGURACION_2 9
#define CONFIGURACION_3 10
#define CONFIGURACION_4 12
#define CONFIGURACION_5 13
#define CONFIGURACION_6 14
#define CONFIGURACION_7 15
#define CONFIGURACION_8 16

#define ESTADISTICAS 4

#define HORA 5
#define HORA_2 6
#define HORA_3 7

//pines
#define PUL 5
#define LUZ 6
#define PIN_BOMBA 7
#define PIN_VENTIS 10
#define LED 11

#define EJE_X A0
#define EJE_Y A1
#define PIN_TIERRA A2
#define TRIGGER 8
#define ECHO 9
#define TIEMPO_MAX 19.52
#define VEL_SONIDO_CM_US 0.0343

//modos y estados
#define AUTOMATICO 0
#define MANUAL 1
#define DIA 1
#define NOCHE 0

//variables
int menu = 0;
bool Modo = 0;
int EstadoLuz;
bool estadoFoco = false;
bool estadoBomba = false;
bool estadoVentis = false;
int horaMinima = 6;
int horaMaxima = 20;
int minimoTierra = 40;
int valorMaximo2 = 560;
int valorMinimo2 = 250;
int valorLeidoHumedad;
int mapeoTierra;
int tiempoRiego = 5;
int tiempoMedicionTierra = 10;
int baseTanque = 25;
int superficieTanque = 5;
int valorMapeado;

//objetos
ThreeWire myWire(3, 4, 2);
RtcDS1302<ThreeWire> Rtc(myWire);
Bounce pul1 = Bounce();
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long tiempoReferencia = 0;
unsigned long tiempoActual;

unsigned long tiempoReferencia_2 = 0;
unsigned long tiempoActual_2;

void setup() {

  //inicializaciones
  lcd.init();
  lcd.backlight();
  Rtc.Begin();
  RtcDateTime fechaYHoraDeCompilacion = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(fechaYHoraDeCompilacion);

  pul1.attach(PUL, INPUT_PULLUP);
  pinMode(EJE_X, INPUT);
  pinMode(EJE_Y, INPUT);
  pinMode(PIN_BOMBA, OUTPUT);
  pinMode(PIN_VENTIS, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  Serial.begin(115200);
  Serial.println("setup ok");
  Serial.println("empezamos en menu MODO");
}

void loop() {

  //lectura joystick
  int X = analogRead(EJE_X);
  int Y = analogRead(EJE_Y);
  pul1.update();
  bool Btn1 = pul1.fell();
  if (Btn1 == true) {
    Serial.println("boton pulsado");
  }

  //obtenemos fecha y hora
  RtcDateTime tiempoActual = Rtc.GetDateTime();
  unsigned int anio = tiempoActual.Year();
  unsigned int hora = tiempoActual.Hour();
  unsigned int mes = tiempoActual.Month();
  unsigned int minutos = tiempoActual.Minute();
  unsigned int dia = tiempoActual.Day();
  unsigned int segundos = tiempoActual.Second();

  //deteccion de ciclo de luz
  if (hora > horaMinima && hora < horaMaxima) {
    EstadoLuz = DIA;
  } else {
    EstadoLuz = NOCHE;
  }

  //lectura sensor humedad de tierra
  valorLeidoHumedad = analogRead(PIN_TIERRA);
  if (valorLeidoHumedad < valorMinimo2) {
    valorMinimo2 = valorLeidoHumedad;
  }
  if (valorLeidoHumedad > valorMaximo2) {
    valorMaximo2 = valorLeidoHumedad;
  }
  mapeoTierra = map(valorLeidoHumedad, valorMinimo2, valorMaximo2, 100, 0);
  mapeoTierra = constrain(mapeoTierra, 0, 100);

  //lectura sensor ultrasonido
  digitalWrite(TRIGGER, true);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, false);

  long tiempoEnMicrosegundos = pulseIn(ECHO, true);
  float tiempoEnMilisegundos = tiempoEnMicrosegundos / 1000.0;

  if (tiempoEnMicrosegundos > 0 && tiempoEnMilisegundos <= TIEMPO_MAX) {
    int distanciaRecorrida = tiempoEnMicrosegundos * VEL_SONIDO_CM_US;
    int distanciaAlObjeto = distanciaRecorrida / 2;
    valorMapeado = map(distanciaAlObjeto, superficieTanque, baseTanque, 100, 0);
    valorMapeado = constrain(valorMapeado, 0, 100);
  }

  if (mapeoTierra < minimoTierra) {
    digitalWrite(LED, true);
  } else {
    digitalWrite(LED,false);
  }

  //control de menues
  tiempoActual = millis();
  if (tiempoActual - tiempoReferencia >= 300) {

    tiempoReferencia = tiempoActual;

    //menu MODO
    if (menu == MODO) {

      lcd.setCursor(0, 0);
      lcd.print("<-    modo   ->");

      if (Modo == AUTOMATICO) {

        lcd.setCursor(3, 1);
        lcd.print("AUTOMATICO");

      } else if (Modo == MANUAL) {

        lcd.setCursor(3, 1);
        lcd.print("  MANUAL  ");
      }


      //cambio de modo
      if (X >= 800 || X <= 200) {
        Modo = !Modo;
        Serial.println("cambio de modo");
      }

      //transicion
      if (Y >= 800) {
        menu++;
        lcd.clear();
        Serial.println("menu SENSORES");
      } else if (Y <= 200) {
        menu = HORA;
        Serial.println("menu HORA");
        lcd.clear();
      }

    }
    //menu SENSORES
    else if (menu == SENSORES) {
      lcd.setCursor(0, 0);
      lcd.print("<-    MENU    ->");
      lcd.setCursor(0, 1);
      lcd.print("    SENSORES    ");



      //transicion
      if (Y >= 800) {
        menu++;
        Serial.println("menu ACTUADORES");
        lcd.clear();
      } else if (Y <= 200) {
        menu--;
        Serial.println("menu MODO");
        lcd.clear();
      }
      if (X < 200) {
        menu = SENSORES_2;
        Serial.println("menu SENSORES_2");
        lcd.clear();
      }

    }
    //menu SENSORES_2
    else if (menu == SENSORES_2) {

      lcd.setCursor(0, 0);
      lcd.print("Hum. Tierra:");
      lcd.print(mapeoTierra);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("Tanque al:");
      lcd.print(valorMapeado);
      lcd.print("%");


      if (X > 800) {
        menu = SENSORES;
        Serial.println("menu SENSORES");
        lcd.clear();
      }

    }
    //menu ACTUADORES
    else if (menu == ACTUADORES) {

      lcd.setCursor(0, 0);
      lcd.print("<-    MENU    ->");
      lcd.setCursor(0, 1);
      lcd.print("   ACTUADORES   ");


      //transicion
      if (Y >= 800) {
        menu++;
        Serial.println("menu CONFIGURACION");
        lcd.clear();
      } else if (Y <= 200) {
        menu--;
        Serial.println("menu SENSORES");
        lcd.clear();
      } else if (X <= 200) {
        menu = ACTUADORES_2;
        Serial.println("menu ACTUADORES_2");
        lcd.clear();
      }


    }
    //menu ACTUADORES_2
    else if (menu == ACTUADORES_2) {


      if (estadoFoco == true) {
        lcd.setCursor(0, 0);
        lcd.print("luces ON ");
      } else if (estadoFoco == false) {
        lcd.setCursor(0, 0);
        lcd.print("luces OFF");
      }

      //cambiar de estado del foco en modo manual
      if (Modo == MANUAL) {
        if (Y < 200 || Y > 800) {
          estadoFoco = !estadoFoco;
          Serial.println("cambio de estado luz");
        }
      }

      //transicion
      if (X >= 800) {
        menu = ACTUADORES;
        Serial.println("menu ACTUADORES");
        lcd.clear();
      } else if (X < 200) {
        menu = ACTUADORES_3;
        Serial.println("menu ACTUADORES_3");
      }
    }
    //menu ACTUADORES_3
    else if (menu == ACTUADORES_3) {


      if (estadoBomba == true) {
        lcd.setCursor(0, 0);
        lcd.print("bomba ON ");
      } else if (estadoBomba == false) {
        lcd.setCursor(0, 0);
        lcd.print("bomba OFF");
      }

      //cambiar de estado del foco en modo manual
      if (Modo == MANUAL) {
        if (Y < 200 || Y > 800) {
          estadoBomba = !estadoBomba;
          Serial.println("cambio de estado bomba");
        }
      }

      //transicion
      if (X >= 800) {
        menu = ACTUADORES_2;
        Serial.println("menu ACTUADORE_2");
      } else if (X < 200) {
        menu = ACTUADORES_4;
        Serial.println("menu ACTUADORES_4");
      }
    }
    //menu ACTUADORES_4
    else if (menu == ACTUADORES_4) {


      if (estadoVentis == true) {
        lcd.setCursor(0, 0);
        lcd.print("Ventis ON ");
      } else if (estadoVentis == false) {
        lcd.setCursor(0, 0);
        lcd.print("Ventis OFF");
      }

      //cambiar de estado del foco en modo manual
      if (Modo == MANUAL) {
        if (Y < 200 || Y > 800) {
          estadoVentis = !estadoVentis;
          Serial.println("cambio de estado Ventis");
        }
      }

      //transicion
      if (X >= 800) {
        menu = ACTUADORES_3;
        Serial.println("menu ACTUADORE_3");
      }




    }

    //menu CONFIGURACION
    else if (menu == CONFIGURACION) {

      lcd.setCursor(0, 0);
      lcd.print("<-    MENU    ->");
      lcd.setCursor(0, 1);
      lcd.print("  CONFIGURACION  ");


      //transicion
      if (Y >= 800) {
        menu++;
        Serial.println("menu ESTADISTICAS");
        lcd.clear();
      } else if (Y <= 200) {
        menu--;
        Serial.println("menu ACTUADORES");
        lcd.clear();
      } else if (X <= 200) {
        menu = CONFIGURACION_2;
        Serial.println("menu CONFIGURACION_2");
        lcd.clear();
      }

    }
    //menu CONFIGURACION_2
    else if (menu == CONFIGURACION_2) {

      lcd.setCursor(0, 0);
      lcd.print("inicio luz:");
      lcd.print(horaMinima);
      lcd.print("hrs");

      //configuracion horario de inicio de luz
      if (Y >= 800) {
        horaMinima = horaMinima + 1;
        Serial.println("subimos horaMinima");
        if (horaMinima > 24) {
          horaMinima = 24;
        }

      } else if (Y <= 200) {
        horaMinima = horaMinima - 1;
        Serial.println("bajamos horaMinima");
        if (horaMinima < 1) {
          horaMinima = 1;
        }
      }


      //transiciones
      if (X >= 800) {
        menu = CONFIGURACION;
        Serial.println("menu CONFIGURACION");
        lcd.clear();
      } else if (X <= 200) {
        menu = CONFIGURACION_3;
        Serial.println("menu CONFIGURACION_2");
        lcd.clear();
      }


    }
    //menu CONFIGURACION_3
    else if (menu == CONFIGURACION_3) {

      lcd.setCursor(0, 0);
      lcd.print("final luz:");
      lcd.print(horaMaxima);
      lcd.print("hrs");

      //configuracion horario de fin de la luz
      if (Y >= 800) {
        horaMaxima = horaMaxima + 1;
        Serial.println("subimos horaMaxima");
        if (horaMaxima > 24) {
          horaMaxima = 24;
        }
      } else if (Y <= 200) {
        horaMaxima = horaMaxima - 1;
        Serial.println("bajamos horaMaxima");
        if (horaMaxima < 1) {
          horaMaxima = 1;
        }
      }

      //transiciones
      if (X >= 800) {
        menu = CONFIGURACION_2;
        Serial.println("menu CONFIGURACION_2");
        lcd.clear();
      } else if (X <= 200) {
        menu = CONFIGURACION_4;
        Serial.println("menu CONFIGURACION_4");
        lcd.clear();
      }

    }
    //menu CONFIGURACION_4
    else if (menu == CONFIGURACION_4) {

      lcd.setCursor(0, 0);
      lcd.print("Min. Humedad:");
      lcd.print(minimoTierra);
      lcd.print("%");

      //configuracion de limite minimo de humedad de tierra
      if (Y >= 800) {
        minimoTierra = minimoTierra + 5;
        Serial.println("subimos minimoTierra");
        if (minimoTierra > 100) {
          horaMaxima = 100;
        }
      } else if (Y <= 200) {
        minimoTierra = minimoTierra - 5;
        Serial.println("bajamos minimoTierra");
        if (minimoTierra < 0) {
          horaMaxima = 0;
        }
      }

      //transiciones
      if (X >= 800) {
        menu = CONFIGURACION_3;
        Serial.println("menu CONFIGURACION_3");
        lcd.clear();
      } else if (X < 200) {
        menu = CONFIGURACION_5;
        Serial.println("menu CONFIGURACION_5");
        lcd.clear();
      }
    }
    //menu CONFIGURACION_5
    else if (menu == CONFIGURACION_5) {

      lcd.setCursor(0, 0);
      lcd.print("Tiempo riego:");
      lcd.print(tiempoRiego);
      lcd.print("s");

      //configuracion de tiempo de riego
      if (Y >= 800) {
        tiempoRiego = tiempoRiego + 1;
        Serial.println("subimos tiempoRiego");
      } else if (Y <= 200) {
        tiempoRiego = tiempoRiego - 1;
        Serial.println("bajamos tiempoRiego");
      }


      if (X >= 800) {
        menu = CONFIGURACION_4;
        Serial.println("menu CONFIGURACION_4");
        lcd.clear();
      } else if (X < 200) {
        menu = CONFIGURACION_6;
        Serial.println("menu CONFIGURACION_6");
        lcd.clear();
      }

    }
    //menu CONFIGURACION_6
    else if (menu == CONFIGURACION_6) {

      lcd.setCursor(0, 0);
      lcd.print("Tiempo medicion");
      lcd.setCursor(0, 1);
      lcd.print("de tierra:");
      lcd.print(tiempoMedicionTierra);
      lcd.print("m");

      //configuracion de tiempo de riego
      if (Y >= 800) {
        tiempoMedicionTierra = tiempoMedicionTierra + 1;
        Serial.println("subimos tiempoMedicionTierra");
      } else if (Y <= 200) {
        tiempoMedicionTierra = tiempoMedicionTierra - 1;
        Serial.println("bajamos tiempoMedicionTierra");
      }



      if (X >= 800) {
        menu = CONFIGURACION_5;
        Serial.println("menu CONFIGURACION_5");
        lcd.clear();
      }else if (X < 200) {
        menu = CONFIGURACION_7;
        Serial.println("menu CONFIGURACION_7");
        lcd.clear();
      }

    }
    //menu CONFIGURACION_7
    else if (menu == CONFIGURACION_7) {

      lcd.setCursor(0, 0);
      lcd.print("Superficie del");
      lcd.setCursor(0, 1);
      lcd.print("tanque a:");
      lcd.print(superficieTanque);
      lcd.print("cm");

      //configuracion de medida del tanque
      if (Y >= 800) {
        superficieTanque = superficieTanque + 1;
        Serial.println("subimos superficieTanque");
      } else if (Y <= 200) {
        superficieTanque = superficieTanque - 1;
        Serial.println("bajamos superficieTanque");
      }

      //transiciones
      if (X >= 800) {
        menu = CONFIGURACION_6;
        Serial.println("menu CONFIGURACION_6");
        lcd.clear();
      } else if (X < 200) {
        menu = CONFIGURACION_8;
        Serial.println("menu CONFIGURACION_8");
        lcd.clear();
      }

    }
    //menu CONFIGURACION_8
    else if (menu == CONFIGURACION_8) {

      lcd.setCursor(0, 0);
      lcd.print("Base del");
      lcd.setCursor(0, 1);
      lcd.print("tanque a:");
      lcd.print(baseTanque);
      lcd.print("cm");

      //configuracion de tiempo de riego
      if (Y >= 800) {
        baseTanque = baseTanque + 1;
        Serial.println("subimos baseTanque");
      } else if (Y <= 200) {
        baseTanque = baseTanque - 1;
        Serial.println("bajamos baseTanque");
      }
      if (X >= 800) {
        menu = CONFIGURACION_7;
        Serial.println("menu CONFIGURACION_7");
        lcd.clear();
      }


    }

    //menu ESTADISTICAS
    else if (menu == ESTADISTICAS) {

      lcd.setCursor(0, 0);
      lcd.print("<-    MENU    ->");
      lcd.setCursor(0, 1);
      lcd.print("  ESTADISTICAS  ");

      //transicion
      if (Y >= 800) {
        menu++;
        Serial.println("menu HORA");
        lcd.clear();
      } else if (Y <= 200) {
        menu--;
        Serial.println("menu CONFIGURACION");
        lcd.clear();
      }

    }
    //menu HORA
    else if (menu == HORA) {

      lcd.setCursor(0, 0);
      lcd.print("HORA    ");
      lcd.print(hora);
      lcd.print(":");
      lcd.print(minutos);
      lcd.print(":");
      lcd.print(segundos);
      lcd.setCursor(0, 1);
      lcd.print("FECHA    ");
      lcd.print(dia);
      lcd.print("/");
      lcd.print(mes);


      //transicion
      if (Y >= 800) {
        menu = MODO;
        Serial.println("menu MODO");
        lcd.clear();
      } else if (Y <= 200) {
        menu--;
        Serial.println("menu ESTADISTICAS");
        lcd.clear();
      } else if (X < 200) {
        Serial.println("menu HORA_2");
        menu = HORA_2;
        lcd.clear();
      }

    }
    //menu HORA_2
    else if (menu == HORA_2) {

      if (EstadoLuz == DIA) {
        lcd.setCursor(0, 0);
        lcd.print("FALTAN");

        lcd.setCursor(0, 1);
        lcd.print(horaMaxima - hora - 1);
        lcd.print("hrs ");
        lcd.print(60 - minutos);
        lcd.print("m de luz");

      } else if (EstadoLuz == NOCHE) {
        lcd.setCursor(0, 0);
        lcd.print("FALTAN");
        lcd.setCursor(0, 1);

        if (hora <= 24) {
          int resto = 24 - hora;
          int horaNueva = hora + resto + 1;
          if (horaNueva > 24) {
            horaNueva = 0;
          }
          lcd.print(horaMinima - horaNueva + resto - 1);
        } else if (hora >= 0) {
          lcd.print(horaMinima - hora - 1);
        }

        lcd.print("hrs ");
        lcd.print(60 - minutos);
        lcd.print("m de noche");
      }

      //transicion
      if (X > 800) {
        Serial.println("menu HORA");
        menu = HORA;
        lcd.clear();
      }
    }
  }

  //actividades en modo automatico
  if (Modo == AUTOMATICO) {

    //luz
    if (EstadoLuz == DIA) {
      estadoFoco = true;
    } else if (EstadoLuz == NOCHE) {
      estadoFoco = false;
    }

    //agua
    tiempoActual_2 = millis();
    if (tiempoActual_2 - tiempoReferencia_2 >= tiempoMedicionTierra * 60000) {
      tiempoReferencia_2 = tiempoActual_2;

      if (mapeoTierra < minimoTierra && EstadoLuz == NOCHE) {

        digitalWrite(estadoBomba, true);
        delay(tiempoRiego * 1000);
        digitalWrite(estadoBomba, false);
      }
    }

    //FIJARSE SI ESTAN BIEN LOS FLAGS O HAY QUE ROTAR
    //actualizamos actuadores
    digitalWrite(LUZ, estadoFoco);
    digitalWrite(PIN_BOMBA, estadoBomba);
    digitalWrite(PIN_VENTIS, estadoVentis);
  }
}