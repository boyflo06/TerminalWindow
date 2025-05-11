SRC_PATH = src/
OBJ_PATH = obj/
INC_PATH = inc/
OUT_PATH = bin/

SRC_FILES = Array2D.cpp color.cpp Vec2i.cpp Window.cpp main.cpp

SRC = $(addprefix ${SRC_PATH}, ${SRC_FILES})
OBJ = $(patsubst $(SRC_PATH)%.cpp, $(OBJ_PATH)%.o, ${SRC})
FINAL = bin/tgraphics

OBJ_DIRS := $(sort $(dir ${OBJ})) 
FLAGS = -Wall -Wextra -Werror -Ofast

all: ${FINAL}

${FINAL}: ${OBJ} | ${OUT_PATH}
	c++ ${FLAGS} -I ${INC_PATH} -o $@ $^ -lncurses

${OBJ_PATH}%.o: ${SRC_PATH}%.cpp | $(OBJ_DIRS)
	c++ ${FLAGS} -I ${INC_PATH} -c $< -o $@ -lncurses

$(OBJ_DIRS):
	mkdir -p $@

${OUT_PATH}:
	mkdir -p ${OUT_PATH}

clean:
	rm -rf ${OBJ_PATH}

fclean: clean
	rm -rf ${OUT_PATH}

re: fclean all

.PHONY: all clean fclean re