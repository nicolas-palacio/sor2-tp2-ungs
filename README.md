# Sistemas Operativos y Redes 2
## Trabajo Práctico 2


Se creó la siguiente topología y, a partir de ella, se generó tráfico de red mediante NS3 para su posterior análisis con Wireshark.

[![N|Solid](https://i.ibb.co/KGPxtYf/Topologia-Informe.jpg)](https://i.ibb.co/KGPxtYf/Topologia-Informe.jpg)


Ver el archivo [TP2-SOR2-Informe.pdf](https://github.com/nicolas-palacio/sor2-tp2-ungs/blob/main/TP2-SOR2-Informe.pdf "informe completo") para conocer más detalles.

### Ejecución de comandos

Para generar los archivos .pcap ejecutar en la carpeta ns3-3.1:
```sh
./waf --run scratch/<nombre_del_archivo.cc>
```
Para generar el gráfico de la cwnd, ejecutar:
```sh
gnuplot config_cwnd.plt
```




