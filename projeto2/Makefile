ENV_NAME = venv

.PHONY: all install run clean

all: install run

install:
	@echo "Criando ambiente virtual e instalando dependências..."
	python3 -m venv $(ENV_NAME)
	. $(ENV_NAME)/bin/activate && pip install --upgrade pip
	. $(ENV_NAME)/bin/activate && pip install -r requirements.txt

run:
	@echo "Executando análise de pacotes..."
	. $(ENV_NAME)/bin/activate && python3 package_analyzer.py

clean:
	rm -rf $(ENV_NAME)
	rm -f *.png
