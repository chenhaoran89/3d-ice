###############################################################################
#                                                                             #
# Makefile                                                                    #
#                                                                             #
# EPFL-STI-IEL-ESL                                                            #
# Bâtiment ELG, ELG 130                                                       #
# Station 11                                                                  #
# 1015 Lausanne, Switzerland                    alessandro.vincenzi@epfl.ch   #
###############################################################################

.PHONY: all clean lib hotspot2threedice

all: lib hotspot2threedice

lib:
	cd ./flex    ; make ;
	cd ./bison   ; make ;
	cd ./sources ; make ;
	cd ./lib     ; make ;

hotspot2threedice: lib
	cd ./hotspot2threedice; make ;

clean:
	cd ./flex              ; make clean ;
	cd ./bison             ; make clean ;
	cd ./sources           ; make clean ;
	cd ./lib               ; make clean ;
	cd ./hotspot2threedice ; make clean ;
