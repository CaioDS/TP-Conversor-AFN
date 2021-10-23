// TP-Conversor-AFN.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include "tinyxml2.h"
#include <vector>
#include <stack>
#include<string>

using namespace tinyxml2;
using namespace std;

typedef enum Alphabet
{
	A = 0,
	B = 1,
};

typedef struct Transition {
	int from;
	int to;
	int read;
} Transition;

typedef struct AFDTransition {
	stack<int>* to;
	int read;
} AFNTransition;

typedef struct AFDState {
	int* id;
	stack<int>* states;
	vector<AFDTransition> *transitions;
	bool initial;
	bool final;
} AFNState;

typedef struct State {
	int id;
	bool initial = false;
	bool final = false;
	vector<Transition> *transitions;
} State;

vector<State>* AFN_automaton;
vector<AFDState>* AFDautomaton;

stack<int> sortStack(stack<int>& input)
{
	stack<int> tmpStack;
	while (!input.empty())
	{
		int tmp = input.top();
		input.pop();

		while (!tmpStack.empty() && tmpStack.top() > tmp)
		{
			input.push(tmpStack.top());
			tmpStack.pop();
		}
		tmpStack.push(tmp);
	}
	return tmpStack;
}

bool compareStacks(stack<int> s1, stack<int> s2)
{
	stack<int> sortedStackS1 = sortStack(s1);
	stack<int> sortedStackS2 = sortStack(s2);

	int N = sortedStackS1.size();
	int M = sortedStackS2.size();

	if (N != M) {
		return false;
	}

	for (int i = 1; i <= N; i++) {
		if (sortedStackS1.top() != sortedStackS2.top())
			return false;

		sortedStackS1.pop();
		sortedStackS2.pop();
	}

	return true;
}

bool checkIfNewStateExists(stack<int> state, vector<AFDState> AFDautomaton) {
	bool result = false;
	for (int i = 0; i < AFDautomaton.size(); i++) {
		AFDState compareState = AFDautomaton.at(i);
		if (compareStacks(state, *compareState.states)) {
			return result = true;
		}
	}

	return result;
}

//FUNÇÃO RESPONSÁVEL POR CRIAR O AUTOMATO
void createAutomaton(XMLDocument *automaton) {
	AFN_automaton = new vector<State>;
	int numberOfStates = 0;
	for (XMLElement* listElement = automaton->FirstChildElement("structure")->FirstChildElement("automaton")->FirstChildElement("state");
		listElement != NULL;
		listElement = listElement->NextSiblingElement("state"))
	{
		const char* stateId = listElement->FindAttribute("id")->Value();
		int convertedStateId = atoi(stateId);
		bool isInitial = false;
		bool isFinal = false;

		if (listElement->FirstChildElement("initial")) {
			isInitial = true;
		}

		if (listElement->FirstChildElement("final")) {
			isFinal = true;
		}

		State* state = new State{ convertedStateId, isInitial, isFinal, new vector<Transition> };
		
		AFN_automaton->push_back(*state);
	}
}

//FUNCÃO RESPONSÁVEL POR ADICIONAR A TRANSIÇÃO LIDA DO ARQUIVO NA ESTRUTURA AFN
void addTransition(int from, int to, int read) {
	State aux = AFN_automaton->at(from);
	Transition transition = Transition{
		from, to, read
	};

	aux.transitions->push_back(transition);
}

//FUNCÃO RESPONSÁVEL POR LER AS TRANSIÇÕES NO ARQUIVO JFF (JFLAP) DE ENTRADA AFN
void readXMLTransitions(XMLDocument* automaton) {
	for (XMLElement* listElement = automaton->FirstChildElement("structure")->FirstChildElement("automaton")->FirstChildElement("transition");
		listElement != NULL;
		listElement = listElement->NextSiblingElement("transition"))
	{
		const char* from = listElement->FirstChildElement("from")->GetText();
		const char* to = listElement->FirstChildElement("to")->GetText();
		const char* read = listElement->FirstChildElement("read")->GetText();

		int fromProperty = atoi(from);
		int toProperty = atoi(to);
		int readProperty = atoi(read);
		
		addTransition(fromProperty, toProperty, readProperty);
	}
}

//FUNCÃO RESPONSÁVEL POR LER O ARQUIVO JFF (JFLAP) DE ENTRADA AFN
void ReadXMLEntryPoint() {
	XMLDocument afnEntryPoint;
	XMLError errorResult = afnEntryPoint.LoadFile("dados/AFNinput.jff");

	if (errorResult != XML_SUCCESS) {
		throw new exception("Ocorreu uma falha ao processar o arquivo de entrada");
	}

	createAutomaton(&afnEntryPoint);
	readXMLTransitions(&afnEntryPoint);
}

//FUNÇÃO REPONSÁVEL POR BUSCAR TRANSIÇÕES NO AFD
AFDTransition searchAFDTransitions(vector<Transition> transitions, Alphabet alphabet) {
	stack<int>* stackTransitions = new stack<int>;

	for (int t = 0; t < transitions.size(); t++) {
		Transition auxTransition = transitions.at(t);
		
		if (auxTransition.read == alphabet) {
			stackTransitions->push(auxTransition.to);
		}
	}

	return AFDTransition{
		stackTransitions, alphabet
	};
}

//FUNÇÃO RESPONSÁVEL POR DEFINIR O ESTADO INICIAL DO AFD CONVERTIDO
AFDState defineAFDInitialState(vector<State> AFNautomaton) {
	AFDState initialState = AFDState{
				new int, new stack<int>, new vector<AFDTransition>
	};

	for (int i = 0; i < AFNautomaton.size(); i++) {
		State aux = AFNautomaton.at(i);

		if (aux.initial) {
			initialState.states->push(aux.id);
			initialState.initial = true;

			vector<Transition>* transitions = aux.transitions;

			initialState.transitions->push_back(searchAFDTransitions(*transitions, A));
			initialState.transitions->push_back(searchAFDTransitions(*transitions, B));

			break;
		}
	}

	return initialState;
}

//FUNÇÃO RESPONSÁVEL POR BUSCAR O PRÓXIMO ESTADO A SER ANALISADO NA CONVERSÃO
stack<int>* findNextAFDState(stack<int> actualState, vector<State> AFNautomaton, Alphabet alphabet) {
	stack<int>* auxState = new stack<int>;

	while (!actualState.empty()) {
		int state = actualState.top();

		for (int j = 0; j < AFNautomaton.size(); j++) {
			State aux = AFNautomaton.at(j);
			if (aux.id == state) {
				stack<int>* foundedTransitions = searchAFDTransitions(*aux.transitions, alphabet).to;

				while (!foundedTransitions->empty()) {
					auxState->push(foundedTransitions->top());
					foundedTransitions->pop();
				}

				break;
			}
		}
		actualState.pop();
	}

	return auxState;
}

//FUNÇÃO RESPONSÁVEL POR DETERMINAR O PRÓXIMO ESTADO
void defineNextAFDState(stack<int> actualState, vector<State> AFNautomaton, vector<AFDState> *afd) {
	AFDState nextState = AFDState{
		new int, new stack<int>, new vector<AFDTransition>
	};

	if (!checkIfNewStateExists(actualState, *AFDautomaton)) {
		*nextState.states = actualState;
		nextState.transitions->push_back(AFDTransition{ findNextAFDState(actualState, AFNautomaton, A), A });
		nextState.transitions->push_back(AFDTransition{ findNextAFDState(actualState, AFNautomaton, B), B });

		afd->push_back(nextState);

		for (int i = nextState.transitions->size() - 1; i >= 0; i--) {
			AFDTransition transition = nextState.transitions->at(i);

			defineNextAFDState(*transition.to, AFNautomaton, afd);
		}
	}
}

//FUNÇÃO INCIAL DA CONVERSÃO
void convertAfnToAfd(vector<State> AFNautomaton) {
	AFDautomaton = new vector<AFDState>;

	AFDState initialState = defineAFDInitialState(AFNautomaton);
	AFDautomaton->push_back(initialState);

	vector<AFDState>* AFD = new vector<AFDState>;

	for (int i = 0; i < initialState.transitions->size(); i++) {
		AFDTransition transition = initialState.transitions->at(i);

		defineNextAFDState(*transition.to, AFNautomaton, AFDautomaton);
	}
}

//FUNÇÃO RESPOSNSÁVEL POR DEFINIR O ESTADOS DE ACEITAÇÃO DO AFD CONVERTIDO
void defineFinalAFDState() {
	int finalStateId;
	for (int i = 0; i < AFN_automaton->size(); i++) {
		State state = AFN_automaton->at(i);

		if (state.final) {
			for (int i = 0; i < AFDautomaton->size(); i++) {
				AFDState afdState = AFDautomaton->at(i);
				stack<int> aux = *afdState.states;
				while (!aux.empty()) {
					if (aux.top() == state.id) {
						afdState.final = true;
					}
					aux.pop();
				}
				*afdState.id = i;
				AFDautomaton->at(i) = afdState;
			}
		}
	}
}

//FUNÇÃO RESPONSÁVEL POR GERAR A SAÍDA DO AFD
void generateXMLOutputAFD() {
	XMLDocument document;
	XMLDeclaration* declaration = document.NewDeclaration();

	XMLElement* structureElement = document.NewElement("structure");

	XMLElement* typeElement = document.NewElement("type");
	XMLText* typeTextElement = document.NewText("fa");
	typeElement->InsertEndChild(typeTextElement);
	structureElement->InsertEndChild(typeElement);

	XMLElement* automatonElement = document.NewElement("automaton");

	for (int i = 0; i < AFDautomaton->size(); i++) {
		AFDState state = AFDautomaton->at(i);
		stack<int> aux = *state.states;

		XMLElement* stateElement = document.NewElement("state");
		string idTex = to_string(*state.id);
		stateElement->SetAttribute("id", idTex.c_str());

		string nameText = "";
		while (!aux.empty()) {
			nameText = nameText + to_string(aux.top()) + " ";
			aux.pop();
		}
		stateElement->SetAttribute("name", nameText.c_str());

		XMLElement* xElement = document.NewElement("x");
		XMLElement* yElement = document.NewElement("y");
		XMLText* coordinateXElement = document.NewText("70.0");
		XMLText* coordinateYElement = document.NewText("70.0");
		xElement->InsertEndChild(coordinateXElement);
		yElement->InsertEndChild(coordinateYElement);
		stateElement->InsertEndChild(xElement);
		stateElement->InsertEndChild(yElement);

		if (state.initial) {
			XMLElement* intitalElement = document.NewElement("initial");
			stateElement->InsertEndChild(intitalElement);
		}

		if (state.final) {
			XMLElement* finalElement = document.NewElement("final");
			stateElement->InsertEndChild(finalElement);
		}

		automatonElement->InsertEndChild(stateElement);
	}

	for (int i = 0; i < AFDautomaton->size(); i++) {
		AFDState state = AFDautomaton->at(i);
		vector<AFDTransition> transitions = *state.transitions;

		for (int t = 0; t < transitions.size(); t++) {
			AFDTransition transition = transitions.at(t);

			for (int j = 0; j < AFDautomaton->size(); j++) {
				AFDState compareState = AFDautomaton->at(j);

				if (compareStacks(*transition.to, *compareState.states)) {
					XMLElement* transitionElement = document.NewElement("transition");

					XMLElement* fromElement = document.NewElement("from");
					XMLText* fromElementText = document.NewText(to_string(*state.id).c_str());
					fromElement->InsertEndChild(fromElementText);

					XMLElement* toElement = document.NewElement("to");
					XMLText* toElementText = document.NewText(to_string(*compareState.id).c_str());
					toElement->InsertEndChild(toElementText);

					XMLElement* readElement = document.NewElement("read");
					XMLText* readElementText = document.NewText(to_string(transition.read).c_str());
					readElement->InsertEndChild(readElementText);

					transitionElement->InsertEndChild(fromElement);
					transitionElement->InsertEndChild(toElement);
					transitionElement->InsertEndChild(readElement);

					automatonElement->InsertEndChild(transitionElement);

					break;
				}
			}
		}
	}

	structureElement->InsertEndChild(automatonElement);

	document.InsertEndChild(structureElement);
	document.InsertFirstChild(declaration);

	document.SaveFile("dados/AFDConvertedOutput.jff");

	cout << "\n \n Arquivo AFD convertido gerado com SUCESSO em 'AFDConvertedOutput.jff' \n \n \n";
}

int main()
{
	cout << "\n*********************** Conversor AFN -> AFD **************************\n \n";
	cout << "Para utilizar corretamente siga as Instruções: \n";
	cout << "1 - Deve ser adicionado a pasta dados o AFN de entrada (.jff) gerado a partir do Jflap, com o seguinte nome 'AFNinput.jff'. (Já existe um de exemplo na pasta)  \n";
	cout << "2 - Após o algoritmo ler o AFN e realizar a conversão, irá ser gerado um novo arquivo (.jff) com os dados do AFD com este nome: 'AFDConvertedOutput.jff' \n \n";

	cout << "Iniciando conversão... \n";

	ReadXMLEntryPoint();

	cout << "AFN de Entrada lido com sucesso... \n";
	convertAfnToAfd(*AFN_automaton);
	defineFinalAFDState();

	cout << "Conversão realizada com sucesso... \n";
	cout << "Gerando Arquivo AFD... \n";
	generateXMLOutputAFD();

	return 0;


}