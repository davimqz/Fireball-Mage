all:
	gcc ./src/*.c -I./include -o Fireball-Mage -lm

run:
	./Fireball-Mage

clean:
	rm Fireball-Mage
