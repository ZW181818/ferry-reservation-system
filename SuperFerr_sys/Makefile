COMPILER = g++
EXEC= superferry
FLAGS= -Wall -std=c++11 -Icontrol -Ientity -Isystem -Iui -o
FILES = main.cpp \
		ui/mainMenu.cpp \
		control/ferryManager.cpp \
		control/reservationManager.cpp \
		control/sailingManager.cpp \
		entity/ferryASM.cpp \
		entity/reservationASM.cpp \
		entity/sailingASM.cpp \
		entity/vehicleASM.cpp \
		system/utilities.cpp

all: $(EXEC) 

$(EXEC): $(FILES)
	@echo "Compiling..."
	@$(COMPILER) $(FLAGS) $(EXEC) $(FILES)
	@echo "Run with ./superferry"

clean:
	@rm -f $(EXEC)