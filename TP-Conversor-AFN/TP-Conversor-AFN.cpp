// TP-Conversor-AFN.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include "tinyxml2.h"
#include <vector>
#include <stack>

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

void pushElements(stack<int> s1, stack<int> s2, int len)
{
	int i = 1;
	while (i <= len) {
		if (s1.size() > 0) {
			s2.push(s1.top());
			s1.pop();
		}
		i++;
	}
}

bool compareStacks(stack<int> s1, stack<int> s2)
{
	int N = s1.size();
	int M = s2.size();

	if (N != M) {
		return false;
	}

	for (int i = 1; i <= N; i++) {
		pushElements(s1, s2, N - i);
		int val = s1.top();
		pushElements(s2, s1, 2 * (N - i));

		if (val != s2.top())
			return false;

		pushElements(s1, s2, N - i);
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

int countNumberOfAFNStates(XMLDocument *automaton) {
	int numberOfStates = 0;
	for (XMLElement* listElement = automaton->FirstChildElement("structure")->FirstChildElement("automaton")->FirstChildElement("state");
		listElement != NULL;
		listElement = listElement->NextSiblingElement("state"))
	{
		numberOfStates++;
	}
	return numberOfStates;
}

void createAutomaton(int size) {
	AFN_automaton = new vector<State>;
	for (int i = 0; i < size; i++) {
		bool initial = false;
		if (i == 0) {
			initial = true;
		}
		State state = State{
			i, initial, false, new vector<Transition>
		};
		AFN_automaton->push_back(state);
	}
}

void addTransition(int from, int to, int read) {
	State aux = AFN_automaton->at(from);
	Transition transition = Transition{
		from, to, read
	};

	aux.transitions->push_back(transition);
}

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

void ReadXMLEntryPoint() {
	XMLDocument afnEntryPoint;
	XMLError errorResult = afnEntryPoint.LoadFile("AFNinput.xml");

	if (errorResult != XML_SUCCESS) {
		throw new exception("Ocorreu uma falha ao processar o arquivo de entrada");
	}
	int statesSize = countNumberOfAFNStates(&afnEntryPoint);

	createAutomaton(statesSize);
	readXMLTransitions(&afnEntryPoint);
}

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

AFDState defineAFDInitialState(vector<State> AFNautomaton) {
	AFDState initialState = AFDState{
				new stack<int>, new vector<AFDTransition>
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



AFDState defineNextAFDState(stack<int> actuaState, vector<State> AFNautomaton, Alphabet alphabet) {
	AFDState nextState = AFDState{
		new stack<int>, new vector<AFDTransition>
	};

	stack<int>* auxState = new stack<int>;


	while (!actuaState.empty()) {
		int state = actuaState.top();
		
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
		actuaState.pop();
	}

	stack<int> auxNewState = *auxState;
	stack<int> aux1;
	stack<int> aux2;

	/*while (!auxNewState.empty()) {
		int state = auxNewState.top();

		for (int j = 0; j < AFNautomaton.size(); j++) {
			State aux = AFNautomaton.at(j);
			if (aux.id == state) {
				stack<int>* a = searchAFDTransitions(*aux.transitions, A).to;
				while (!a->empty()) {
					aux1.push(a->top());
					a->pop();
				}

				stack<int>* b = searchAFDTransitions(*aux.transitions, B).to;
				while (!a->empty()) {
					aux2.push(b->top());
					b->pop();
				}

				break;
			}
		}
		auxNewState.pop();
	}*/
	                          
	nextState.states = auxState;
	//nextState.transitions->push_back(AFDTransition{ &aux1, A });
	//nextState.transitions->push_back(AFDTransition{ &aux2, B });

	for (int i = 0; i < nextState.transitions->size(); i++) {
		AFDTransition transition = nextState.transitions->at(i);

		if (!checkIfNewStateExists(*nextState.states, *AFDautomaton)) {
			AFDautomaton->push_back(defineNextAFDState(*transition.to, AFNautomaton, alphabet));
			//AFDautomaton->push_back(defineNextAFDState(*transition.to, AFNautomaton, alphabet));
		}
	}
	

	return nextState;

}

vector<AFDState> convertAfnToAfd(vector<State> AFNautomaton) {
	AFDautomaton = new vector<AFDState>;

	AFDState initialState = defineAFDInitialState(AFNautomaton);
	AFDautomaton->push_back(initialState);

	for (int i = 0; i < initialState.transitions->size(); i++) {
		AFDTransition transition = initialState.transitions->at(i);

		AFDState nextState = defineNextAFDState(*transition.to, AFNautomaton, A);
		if (!checkIfNewStateExists(*nextState.states, *AFDautomaton)) {
			AFDautomaton->push_back(nextState);
		}

		AFDState nextState2 = defineNextAFDState(*transition.to, AFNautomaton, B);
		if (!checkIfNewStateExists(*nextState2.states, *AFDautomaton)) {
			AFDautomaton->push_back(nextState2);
		}
	}

	return *AFDautomaton;

}
int main()
{
	try
	{
		ReadXMLEntryPoint();
		convertAfnToAfd(*AFN_automaton);
		return 0;
	}
	catch (const exception x)
	{
		cout << "ERRO";
		return 0;
	}

}