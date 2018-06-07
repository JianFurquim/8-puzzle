#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>
#include <cmath>
#include <unordered_map>
#include <algorithm>

/*
*	Autor: Jian Macedo Furquim 161152063
*	Contato: jian_mf@hotmail.com
*	Descrição: 8-puzzle usando busca em profundidade e busca em largura
*   Compilar: g++ -std=c++11 8-puzzle.c -o main
*/

using namespace std;
#define PUZZLESIZE 9

//class Tabela hash pra ser usado nas buscas
class Hashtable
{
	std::unordered_map<const void*, const void*> htmap;

 public:
	//Insere elemento na hash
	void put(const void* key, const void* value)
	{
		htmap[key] = value;
	}

	//Vê tal elemento ja está na hash
	bool get(const void* key)
	{
		std::unordered_map<const void*, const void*>::const_iterator temp = htmap.find(key);

		if(temp == htmap.end()) return false;
		else return true;
	}

	//limpa a hash
	void clean()
	{
		htmap.clear();
	}
};

//Struct Nodo
struct node
{
	int *state;
	int cost;
	int d;

};
typedef struct node Node;

//Função para criar um novo nodo
Node* createNode(int *state, int depth, int cost)
{
	Node *g = (Node*) malloc(sizeof(Node));
	g->state = state;
	g->cost = cost;
	g->d = depth;
	return g;
}

//Função para printar um estado
void printState(int *state)
{
	int i;
	for(i = 0; i<PUZZLESIZE; i++)
	{
		printf("%d ", state[i]);
		if(i+1 == 3 || i+1 == 6) printf("\n");
	}
	printf("\n");
}

//Função para encontrar o vazio
int isBlank(int* currentState)
{
	for(int i=0; i<PUZZLESIZE; i++)
	{
		if(currentState[i] == 0) return i;
	}

	printf("Error no blank\n");
	exit(1);
}

 //Função para testar se dois estados são iguais
bool isEqual(int* currentState, int* finishState)
{
	for(int i=0; i<PUZZLESIZE-1; i++)
	{
		if(currentState[i] != finishState[i]) return false;
	}
	return true;
}

//Função para gerar chave baseando-se em um estado
int createKeyHash(int* state)
{
	int primo[PUZZLESIZE] = {3, 5, 7, 11, 13, 15, 17, 19, 23};
	int hash=0;

	for(int i=0; i<PUZZLESIZE; i++)
	{
		hash += pow(primo[i], PUZZLESIZE - i) * state[i];
	}
	return hash;
}

 //Função para gerar estados basendo-se no pai
int* createState(int* currentState, int blank, int action)
{
	int i, aux;

	//Move o branco para esquerda
	if(action == 3)
	{
		if(blank % 3 > 0)
		{
			int* newState = (int*) malloc(sizeof(int)*(PUZZLESIZE));

			for(i = 0; i < PUZZLESIZE; i++)
			{
				newState[i] = currentState[i];
			}

			aux = newState[blank - 1];
			newState[blank-1] = 0;
			newState[blank] = aux;
			return newState;
		}
	}
	//Move o branco para direita
	else if(action == 4)
	{
		if(blank < PUZZLESIZE-1 && (blank % 3) < 2 )
		{
			int* newState = (int*) malloc(sizeof(int)*(PUZZLESIZE));

			for(i = 0; i < PUZZLESIZE; i++)
			{
				newState[i] = currentState[i];
			}

			aux = newState[blank + 1];
			newState[blank+1] = 0;
			newState[blank] = aux;
			return newState;
		}
	}

	//Move o branco para cima
	else if(action == 1)
	{
		if(blank - 3 >= 0)
		{
			int* newState = (int*) malloc(sizeof(int)*(PUZZLESIZE));

			for(i = 0; i < PUZZLESIZE; i++)
			{
				newState[i] = currentState[i];
			}

			aux = newState[blank - 3];
			newState[blank-3] = 0;
			newState[blank] = aux;
			return newState;
	 	}
	}
	//Move o branco para baixo
	else if(action == 2)
	{
		if (blank + 3 < PUZZLESIZE)
		{
			int* newState = (int*) malloc(sizeof(int)*(PUZZLESIZE));

			for(i = 0; i < PUZZLESIZE; i++)
			{
				newState[i] = currentState[i];
			}

			aux = newState[blank + 3];
			newState[blank+3] = 0;
			newState[blank] = aux;
			return newState;
	 	}
	}
	return NULL;
}

//Função para gerar filhos de um estado
vector<int*> createSons(int* currentState)
{
	vector<int*> sons;
	for(int i=1; i<5 ; i++)
	{
		int* aux = createState(currentState, isBlank(currentState), i);

		if(aux != NULL)
		{
			sons.push_back(aux);
		}
	}
	return sons;
}

/*	A heuristica funciona de maneira que ela separa os estados em dois blocos, o bloco de cima (de 1 a 6)
	e o de baixo (de 4 a 9), no primeiro if ela acha em que bloco de indices se encontra o valor atual? em cima ou em baixo
	depois encontra-se em que bloco de indices está a posição certa pra ele	no if do if ? em cima ou embaixo
	feito isso ai chegamos na parte em que se tem 4 possibilidade se o indice que ele ta (index[i]) e impar ou par
	e se indice para onde ele deve ir(indCorrect) é impar ou par.

				    1 - Se onde ele ta é no bloco de cima e para onde ele deve ir também (os dois no mesmo bloco)
	| I | P | I |	de par pra par ou de impar pra impar custa 2 e de par pra impar ou visse verssa custa 1 ou 3 no pior caso
	| P | I | P |	2 - Mas se onde ele está é no bloco de cima e para onde ele deve ir é no de baixo (em blocos diferentes)
	| I | P | I |	de par pra par ou de impar pra impar custa 2 ou 4 e de par pra impar custa  3 no pior caso

	no fim basicamente ele soma todos os custos dos que se encontram na posição errada e isso é aproximadamente o
	custo nescessario para chega no estado final.                      */

//Função para gerar o custo de um estado
int generatesCost(int* currentState, int* finishState)
{
	//vetor pra salvar indice dos que estão na posição errada
	vector<int> index;
	//vetor para salvar os valores dos que estão na posição errada
	vector<int> value;
	int cost=0;

	//for pra salvar os errados nos seus respectivos vetores
	for(int i=0; i<PUZZLESIZE; i++)
	{
		if(currentState[i] != finishState[i])
		{
			index.push_back(i+1);
			value.push_back(currentState[i]);
		}
	}

	for(int i=0; i<value.size(); i++)
	{
		//variavél para salvar o indice correto do bloco atual
		int indCorrect = value[i]+1;

		if(index[i] >= 1 && index[i] <= 6)
		{
			if(indCorrect >=1 && indCorrect <= 6)
			{
				if(index[i] % 2 == 0 && indCorrect % 2 == 0) cost +=2;
				else if(index[i] % 2 == 0 && indCorrect % 2 != 0) cost +=1;
				else if(index[i] % 2 != 0 && indCorrect % 2 == 0) cost +=1;
				else if(index[i] % 2 != 0 && indCorrect % 2 != 0) cost +=2;
			}
			else
			{
				if(index[i] % 2 == 0 && indCorrect % 2 == 0) cost +=2;
				else if(index[i] % 2 == 0 && indCorrect % 2 != 0) cost +=3;
				else if(index[i] % 2 != 0 && indCorrect % 2 == 0) cost +=3;
				else if(index[i] % 2 != 0 && indCorrect % 2 != 0) cost +=4;
			}

		}
		else
		{
			if(indCorrect >=1 && indCorrect <= 6)
			{
				if(index[i] % 2 == 0 && indCorrect % 2 == 0) cost +=2;
				else if(index[i] % 2 == 0 && indCorrect % 2 != 0) cost +=3;
				else if(index[i] % 2 != 0 && indCorrect % 2 == 0) cost +=3;
				else if(index[i] % 2 != 0 && indCorrect % 2 != 0) cost +=4;
			}
			else
			{
				if(index[i] % 2 == 0 && indCorrect % 2 == 0) cost +=2;
				else if(index[i] % 2 == 0 && indCorrect % 2 != 0) cost +=1;
				else if(index[i] % 2 != 0 && indCorrect % 2 == 0) cost +=1;
				else if(index[i] % 2 != 0 && indCorrect % 2 != 0) cost +=2;
			}
		}
	}

	index.clear();
	value.clear();
	return cost;
}

//Busca em largura
void searchWidth(Node *startNode, int* finishState)
{
	int cont=0;
	int* currentState;
	deque<Node*> row;
	Hashtable ht;
	vector<int*> sons;

	row.push_back(startNode);
	ht.put((int *)createKeyHash(startNode->state), (int *)startNode->state);

	while(!row.empty())
	{
		Node* aux = row.front();
		currentState = aux->state;
		cont++;
		row.pop_front();

		if(isEqual(currentState, finishState))
		{
			printf("Search Width: \n");
			printf("Current State: \n");
			printState(currentState);
			printf("Width: %d\n", cont);
			printf("\n");
			ht.clean();
			return;
		}

		sons = createSons(currentState);
		for(int i=0; i<sons.size(); i++)
		{
			if(!ht.get((int *)createKeyHash(sons[i])))
			{
				ht.put((int *)createKeyHash(sons[i]), (int *)sons[i]);
				Node* baby = createNode(sons[i], aux->d+1, generatesCost(sons[i], finishState));

				if(baby->cost <= 10) row.push_front(baby);
				else row.push_back(baby);
			}

		}
		sons.clear();
	}

	printf("Not found state! \n");
	ht.clean();
	return;
}

//Busca em profundidade
void searchDepth(Node *startNode, int* finishState)
{
	int* currentState;
	deque<Node*> stack;
	Hashtable ht;
	vector<int*> sons;

	stack.push_front(startNode);
	ht.put((int *)createKeyHash(startNode->state), (int *)startNode->state);

	while(!stack.empty())
	{
		Node* aux = stack.front();
		currentState = aux->state;

		stack.pop_front();

		if(isEqual(currentState, finishState))
		{
			printf("Search Depth: \n");
			printf("Current State: \n");
			printState(currentState);
			printf("Depth: %d\n", aux->d);
			printf("\n");
			ht.clean();
			return;
		}

		//if para limitar a profundidade
		if(aux->d <= 40) sons= createSons(currentState);

		for(int i=0; i<sons.size(); i++)
		{
			if(!ht.get((int *)createKeyHash(sons[i])))
			{
				ht.put((int *)createKeyHash(sons[i]), (int *)sons[i]);

				Node* baby = createNode(sons[i], aux->d+1, generatesCost(sons[i], finishState));

				if(baby->cost <= 10) stack.push_front(baby);
				else stack.push_back(baby);
			}

		}
		sons.clear();
	}

	printf("Not found state! \n");
	ht.clean();
	return;
}

int main()
{
	//Estado inicial
	int startState[PUZZLESIZE] = {6, 4, 3, 5, 1, 2, 7, 8, 0};
	//Estado Final
	int finishState[PUZZLESIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

	//cria nodo inicial
	Node *startNode = createNode(startState, 0, generatesCost(startState, finishState));

	printf("Current State: \n");
	printState(startState);
	printf("\n");

	//Busca em largura
	searchWidth(startNode, finishState);
	//Busca em profundidade
	searchDepth(startNode, finishState);
}
