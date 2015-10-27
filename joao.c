/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Estruturas de Dados 2
 * Prof. Tiago A. de Almeida
 *
 * Trabalho 02 - Árvore B
 *
 * RA: 609684
 * Aluno: João Paulo Bologna
 * ========================================================================== */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Tamanho dos campos dos registros */
#define TAM_PRIMARY_KEY 9
#define TAM_EQUIPE 40
#define TAM_DATA 11
#define TAM_DURACAO 6
#define TAM_PLACAR 3
#define TAM_MVP 40

#define TAM_REGISTRO 192
#define MAX_REGISTROS 1000
#define TAM_ARQUIVO (MAX_REGISTROS * TAM_REGISTRO + 1)

#define TRUE 1
#define FALSE 0

/* Saídas do usuário */
#define OPCAO_INVALIDA "Opcao invalida!\n"
#define MEMORIA_INSUFICIENTE "Memoria insuficiente!"
#define REGISTRO_N_ENCONTRADO "Registro nao encontrado!\n\n"
#define CAMPO_INVALIDO "Campo invalido! Informe novamente.\n"
#define ERRO_PK_REPETIDA "ERRO: Ja existe um registro com a chave primaria: %s.\n"
#define ARQUIVO_VAZIO "Arquivo vazio!"
#define NOS_PERCORRIDOS "Busca por %s. Nos percorridos:\n"

/* Registro da partida */
typedef struct {
	char pk[TAM_PRIMARY_KEY];
	char equipe_azul[TAM_EQUIPE];
	char equipe_vermelha[TAM_EQUIPE];
	char data_partida[TAM_DATA];	// DD/MM/AAAA
	char duracao[TAM_DURACAO];			// MM:SS
	char vencedor[TAM_EQUIPE];
	char placar1[TAM_PLACAR];
	char placar2[TAM_PLACAR];
	char mvp[TAM_MVP];
} Partida;

/* Registro da Árvore-B
 * Contém a chave primária e o RRN do respectivo registro */
typedef struct {
	char pk[TAM_PRIMARY_KEY];	// chave primária
	int rrn;					// rrn do registro
} Chave;

/* Estrutura da Árvore-B */
typedef struct node node_Btree;
struct node {
	int num_chaves;		// numero de chaves armazenadas
	Chave *chave;		// vetor das chaves e rrns [m-1]
	node_Btree **desc;	// ponteiros para os descendentes, *desc[m]
	int folha;			// flag folha da arvore
};
typedef struct {
	node_Btree *raiz;
} Iprimary;

/* Registro do índice secundário
 * Contém o nome da equipe vencedora e a chave primária do registro */
typedef struct {
	char vencedor[TAM_EQUIPE];
	char pk[TAM_PRIMARY_KEY];
} Iwinner;

/* Registro do índice secundário
 * Contém o apelido do MVP e a chave primária do registro */
typedef struct {
	char mvp[TAM_MVP];
	char pk[TAM_PRIMARY_KEY];
} Imvp;

/* Variáveis globais */
char ARQUIVO[TAM_ARQUIVO];
int M;

typedef int bool;

/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Recebe do usuário uma string simulando o arquivo completo e retorna o número
 * de registros. */
int carregar_arquivo();

/* Exibe o jogador */
void exibir_registro(int rrn);

/* <<< DECLARE AQUI OS PROTOTIPOS >>> */

void criar_iprimary(Iprimary *iprimary, int nregistros, int ordem);
void criar_iwinner(Iwinner *iwinner, int nregistros);
void criar_imvp(Imvp *imvp, int nregistros);

void cadastrar(Iprimary *iprimary, Iwinner *iwinner, Imvp *imvp, int *nregistros);
void alterar(Iprimary iprimary);
void buscar(Iprimary iprimary, Iwinner *iwinner, Imvp *imvp, int nregistros);
void listar(Iprimary iprimary, Iwinner *iwinner, Imvp *imvp, int nregistros);
void apagar_no(node_Btree **raiz);
void ignore();
Partida recuperar_registro(int rrn);

int cmpIndicesMvp(const void *a, const void *b);
int cmpIndicesWinner(const void *a, const void *b);
Chave* busca(node_Btree *raiz, const char *pk, int nregistros);
void divide_no(node_Btree *x, Chave k, node_Btree **filho_direito, Chave **chave_promovida);
void insere_aux(node_Btree *x, Chave k, node_Btree **filho_direito, Chave **chave_promovida);
void insere(Iprimary *iprimary, Chave k, Chave **chave_promovida, node_Btree **filho_direito);
node_Btree* cria_no();
bool verificaTamanho(char c[]);
bool verificaData(char data_partida[]);
bool verificaDuracao(char duracao[]);
bool verificaAbates(char abates[]);
bool verificaEquipeVencedora(char equipe_vermelha[], char equipe_azul[], char vencedor[]);
void criaChavePrimaria(Partida *p);
Chave* buscaPK(node_Btree *raiz, const char *pk, int nregistros);
int buscaSec(char *chave, int tipo, Iwinner *winner, Imvp *mvp, int *ini, int *fim, int nItens);
int buscaBinariaWinner(Iwinner *p, int ini, int fim, char elem[]);
int buscaBinariaMvp(Imvp *p, int ini, int fim, char elem[]);
void listaNivel(node_Btree *raiz, int nregistros, int nivel);
void bubleSortWinner(Iwinner *mvp, int nregistros);
void bubleSortMvp(Imvp *mvp, int nregistros);

/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */
int main() {

	/* Arquivo */
	int carregarArquivo = 0, nregistros = 0;
	scanf("%d\n", &carregarArquivo); // 1 (sim) | 0 (nao)
	if (carregarArquivo) {
		nregistros = carregar_arquivo();
	}


	/* Índice primário */
	int ordem;
	scanf("%d", &ordem);
	Iprimary iprimary;
	criar_iprimary(&iprimary, nregistros, ordem);


	/* Índice secundário de vencedores */
	Iwinner *iwinner = (Iwinner *) malloc (MAX_REGISTROS * sizeof(Iwinner));
	if (!iwinner) {
		perror(MEMORIA_INSUFICIENTE);
		exit(1);
	}
	criar_iwinner(iwinner, nregistros);


	/* Índice secundário de MVPs */
	Imvp *imvp = (Imvp *) malloc (MAX_REGISTROS * sizeof(Imvp));
	if (!imvp) {
		perror(MEMORIA_INSUFICIENTE);
		exit(1);
	}
	criar_imvp(imvp, nregistros);


	/* Execução do programa */
	int opcao = 0;
	while(opcao != 5) {
		scanf("%d", &opcao);
		switch(opcao) {

		case 1:
			getchar();
			cadastrar(&iprimary, iwinner, imvp, &nregistros);
			break;
		case 2:
			getchar();
			alterar(iprimary);
			break;
		case 3:
			buscar(iprimary, iwinner, imvp, nregistros);
			break;
		case 4:
			listar(iprimary, iwinner, imvp, nregistros);
			break;

		case 5: /* Libera memoria alocada */
			apagar_no(&iprimary.raiz);
			free(iwinner);
			free(imvp);
			break;

		case 10:
			printf("%s\n", ARQUIVO);
			break;

		default:
			ignore();
			printf(OPCAO_INVALIDA);
			break;
		}
	}
	return 0;
}


/* ==========================================================================
 * ================================= FUNÇÕES ================================
 * ========================================================================== */

/* Recebe do usuário uma string simulando o arquivo completo e retorna o número
 * de registros. */
int carregar_arquivo() {
	scanf("%[^\n]\n", ARQUIVO);
	return strlen(ARQUIVO) / TAM_REGISTRO;
}

/* Exibe a partida */
void exibir_registro(int rrn) {

	Partida j = recuperar_registro(rrn);

	printf("%s\n", j.pk);
	printf("%s\n", j.equipe_azul);
	printf("%s\n", j.equipe_vermelha);
	printf("%s\n", j.data_partida);
	printf("%s\n", j.duracao);
	printf("%s\n", j.vencedor);
	printf("%s\n", j.placar1);
	printf("%s\n", j.placar2);
	printf("%s\n", j.mvp);
	printf("\n");
}

/* <<< IMPLEMENTE AQUI AS FUNCOES >>> */

void criar_iprimary(Iprimary *iprimary, int nregistros, int ordem) {
	char *arqDados = ARQUIVO;
	Partida p;
	int i; 
	Chave k;
	M = ordem;
	Chave *chave_promovida = NULL;
	node_Btree *filho_direito = NULL;

	iprimary->raiz = NULL;
	
	for(i = 0; i < nregistros; i++) {
		sscanf(arqDados, "%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@", p.pk, p.equipe_azul, p.equipe_vermelha, p.data_partida, p.duracao, p.vencedor, p.placar1, p.placar2, p.mvp);

		strcpy(k.pk, p.pk);
		k.rrn = i*192;

		chave_promovida = NULL;
		filho_direito = NULL;

		insere(iprimary, k, &chave_promovida, &filho_direito);
		arqDados += 192;
	}	
}

void criar_iwinner(Iwinner *iwinner, int nregistros) {
	char *arqDados = ARQUIVO;
	Partida p;
	int i; 
	
	for(i = 0; i < nregistros; i++) {
		sscanf(arqDados, "%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@", p.pk, p.equipe_azul, p.equipe_vermelha, p.data_partida, p.duracao, p.vencedor, p.placar1, p.placar2, p.mvp);

		strcpy(iwinner[i].vencedor, p.vencedor);
		strcpy(iwinner[i].pk, p.pk);
		
		arqDados += 192;
	}

	bubleSortWinner(iwinner, nregistros);
}

void criar_imvp(Imvp *imvp, int nregistros) {
	char *arqDados = ARQUIVO;
	Partida p;
	int i; 
	
	for(i = 0; i < nregistros; i++) {
		sscanf(arqDados, "%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@", p.pk, p.equipe_azul, p.equipe_vermelha, p.data_partida, p.duracao, p.vencedor, p.placar1, p.placar2, p.mvp);

		strcpy(imvp[i].mvp, p.mvp);
		strcpy(imvp[i].pk, p.pk);
		
		arqDados += 192;
	}

	bubleSortMvp(imvp, nregistros);
}

void cadastrar(Iprimary *iprimary, Iwinner *iwinner, Imvp *imvp, int *nregistros) {
	Partida p;
	int i;
	bool isCorrect = TRUE;
	Chave *chave_promovida = NULL; 
	node_Btree *filho_direito = NULL;
	Chave k;
	char arqDados[193];

	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.equipe_azul);
	}while(!(isCorrect = verificaTamanho(p.equipe_azul)));
	
	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.equipe_vermelha);
	}while(!(isCorrect = verificaTamanho(p.equipe_vermelha)));
	
	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.data_partida);
	}while(!(isCorrect = verificaData(p.data_partida)));
	
	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.duracao);
	}while(!(isCorrect = verificaDuracao(p.duracao)));
	
	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.vencedor);
	}while(!(isCorrect = verificaEquipeVencedora(p.equipe_azul, p.equipe_vermelha, p.vencedor)));
	
	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.placar1);
	}while(!(isCorrect = verificaAbates(p.placar1)));

	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.placar2);
	}while(!(isCorrect = verificaAbates(p.placar2)));
	
	do{
		isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
		scanf(" %[^\n]", p.mvp);
	}while(!(isCorrect = verificaTamanho(p.mvp)));
	
	criaChavePrimaria(&p);

	k.rrn = *nregistros * 192;
	strcpy(k.pk, p.pk);

	if(busca(iprimary->raiz, p.pk, *nregistros) != NULL)
		printf(ERRO_PK_REPETIDA, p.pk);
	else {
		insere(iprimary, k, &chave_promovida, &filho_direito);

		strcpy(iwinner[*nregistros].pk, p.pk);
		strcpy(iwinner[*nregistros].vencedor, p.vencedor);
		strcpy(imvp[*nregistros].pk, p.pk);
		strcpy(imvp[*nregistros].mvp, p.mvp);

		sprintf(arqDados, "%s@%s@%s@%s@%s@%s@%s@%s@%s@", p.pk, p.equipe_azul, p.equipe_vermelha, p.data_partida, p.duracao, p.vencedor, p.placar1, p.placar2, p.mvp);

		for(i = strlen(arqDados); i < 192; i++){
			arqDados[i] = '#';
		}

		arqDados[i] = '\0';

		strcat(ARQUIVO, arqDados);

		*nregistros = *nregistros + 1;

		bubleSortWinner(iwinner, *nregistros);
		bubleSortMvp(imvp, *nregistros);
	}
}

void alterar(Iprimary iprimary) {
    int deslocamento = 0;
    char *arqDados = ARQUIVO;
    Partida p;
    Chave *k;
    bool isCorrect = TRUE;

    scanf("%s", p.pk);
    k = busca(iprimary.raiz, p.pk, 1);
    
    if(k == NULL)
        printf(REGISTRO_N_ENCONTRADO);
    else {
        do{
			isCorrect == FALSE ? printf(CAMPO_INVALIDO) : (isCorrect = TRUE);
			scanf(" %[^\n]", p.duracao);
		}while(!(isCorrect = verificaDuracao(p.duracao)));
        
        arqDados += k->rrn;
    
		sscanf(arqDados, "%[^@]@%[^@]@%[^@]@%[^@]@", p.pk, p.equipe_azul, p.equipe_vermelha, p.data_partida);
		deslocamento = deslocamento + strlen(p.pk) + strlen(p.equipe_azul) + strlen(p.equipe_vermelha) + strlen(p.data_partida) + 4;

		arqDados[deslocamento] = p.duracao[0];
		arqDados[deslocamento+1] = p.duracao[1];
		arqDados[deslocamento+3] = p.duracao[3];
		arqDados[deslocamento+4] = p.duracao[4];
	}
}

void buscar(Iprimary iprimary, Iwinner *iwinner, Imvp *imvp, int nregistros) {
	int opcao;
	int i, ini = 0, fim = 0, posicao = -1;
	char pk[9], vencedor[TAM_EQUIPE], mvp[TAM_MVP];
	Chave *k;

	scanf("%d", &opcao);

	switch(opcao) {
		case 1:
			if(nregistros == 0)
				printf(ARQUIVO_VAZIO);
			else {
				scanf("%s", pk);
				printf(NOS_PERCORRIDOS, pk);
				k = buscaPK(iprimary.raiz, pk, nregistros);
				printf("\n");
				
				if(k == NULL)
					printf(REGISTRO_N_ENCONTRADO);
				else
				{
					printf("\n");
					exibir_registro(k->rrn);
				}
			}
		break;
		case 2:
			if(nregistros == 0)
				printf(ARQUIVO_VAZIO);
			else {
				scanf(" %[^\n]", vencedor);
				posicao = buscaSec(vencedor, 1, iwinner, imvp, &ini, &fim, nregistros);

				if(posicao == -1)
					printf(REGISTRO_N_ENCONTRADO);
				else
				{
					for(i = ini; i < fim; i++)
					{
						k = busca(iprimary.raiz, iwinner[i].pk, nregistros);

						exibir_registro(k->rrn);
					}
				}
			}
		break;
		case 3:
			if(nregistros == 0)
				printf(ARQUIVO_VAZIO);
			else {
				scanf(" %[^\n]", mvp);
				posicao = buscaSec(mvp, 2, iwinner, imvp, &ini, &fim, nregistros);

				if(posicao == -1)
					printf(REGISTRO_N_ENCONTRADO);
				else
				{
					for(i = ini; i < fim; i++)
					{
						k = busca(iprimary.raiz, imvp[i].pk, nregistros);

						exibir_registro(k->rrn);
					}
				}
			}
		break;
		default:
			printf(OPCAO_INVALIDA);
			ignore();
		break;
	}
}

void listar(Iprimary iprimary, Iwinner *iwinner, Imvp *imvp, int nregistros) {
	int opcao;
	int i;
	Chave *k;

	scanf("%d", &opcao);

	switch(opcao) {
		case 1:
			if(nregistros == 0)
				printf(ARQUIVO_VAZIO);
			else {
				listaNivel(iprimary.raiz, nregistros, 1);
				printf("\n");
			}
		break;
		case 2:
			if(nregistros == 0)
				printf(ARQUIVO_VAZIO);
			else {

				for(i = 0; i < nregistros; i++) {
					k = busca(iprimary.raiz, iwinner[i].pk, nregistros);

					exibir_registro(k->rrn);
				}
			}
		break;
		case 3:
			if(nregistros == 0)
				printf(ARQUIVO_VAZIO);
			else {

				for(i = 0; i < nregistros; i++) {
					k = busca(iprimary.raiz, imvp[i].pk, nregistros);

					exibir_registro(k->rrn);
				}
			}
		break;
		default:
			printf(OPCAO_INVALIDA);
			ignore();
		break;
	}
}

void apagar_no(node_Btree **raiz) {
	int i = 0;

	if(*raiz == NULL)
		return;

	if((*raiz)->folha == 1)
		free(*raiz);
	else {
		for(i = 0; i < (*raiz)->num_chaves + 1; i++)
			apagar_no(&(*raiz)->desc[i]);
	}
}

void ignore() {
	char c;
   	while ((c = getchar()) != '\n' && c != EOF);
}

Partida recuperar_registro(int rrn) {
	char *arqDados = ARQUIVO;
	Partida p;

	arqDados += rrn;
	sscanf(arqDados, "%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@", p.pk, p.equipe_azul, p.equipe_vermelha, p.data_partida, p.duracao, p.vencedor, p.placar1, p.placar2, p.mvp);
	return p;
}

//Compara indices secundário a partir do strcmp
int cmpIndicesMvp(const void *a, const void *b) {
	const Imvp *s1 = a;
    const Imvp *s2 = b;
    if(strcasecmp(s1->mvp, s2->mvp) == 0)
    	return strcasecmp(s1->pk, s2->pk);
     return strcmp(s1->mvp, s2->mvp);
}

//Compara indices secundário a partir do strcmp
int cmpIndicesWinner(const void *a, const void *b) {
	const Iwinner *s1 = a;
    const Iwinner*s2 = b;
    if(strcasecmp(s1->vencedor, s2->vencedor) == 0)
    	return strcasecmp(s1->pk, s2->pk);
     return strcmp(s1->vencedor, s2->vencedor);
}

Chave* busca(node_Btree *raiz, const char *pk, int nregistros) {
	int i = 0;

	if(nregistros == 0)
		return NULL;

	if(raiz == NULL)
		return NULL;

	while(i < raiz->num_chaves && strcmp(pk, raiz->chave[i].pk) > 0)
		i++;

	if(i < raiz->num_chaves && strcmp(pk, raiz->chave[i].pk) == 0)
		return &(raiz->chave[i]);

	if(raiz->folha == 1)
		return NULL;
	else
		return busca(raiz->desc[i], pk, nregistros);
}

Chave* buscaPK(node_Btree *raiz, const char *pk, int nregistros) {
	int i = 0, j;

	if(nregistros == 0)
		return NULL;

	while(i < raiz->num_chaves && strcmp(pk, raiz->chave[i].pk) > 0) {
		i++;
	}

	for(j = 0; j < raiz->num_chaves; j++)
		if(j == 0)
			printf("%s", raiz->chave[j].pk);
		else
			printf(", %s", raiz->chave[j].pk);

	if(i < raiz->num_chaves && strcmp(pk, raiz->chave[i].pk) == 0) {
		return &(raiz->chave[i]);
	}

	printf("\n");

	if(raiz->folha == 1)
		return NULL;
	else
		return buscaPK(raiz->desc[i], pk, nregistros);
}

void listaNivel(node_Btree *raiz, int nregistros, int nivel) {
	int i = 0;

	if(nregistros == 0)
		return;

	printf("%d - ", nivel);

	while(i < raiz->num_chaves) {
		if(i == 0)
			printf("%s", raiz->chave[i].pk);
		else
			printf(", %s", raiz->chave[i].pk);
		i++;
	}

	printf("\n");

	nivel++;

	if(raiz->folha == 1)
		return;
	else {
		for(i = 0; i < raiz->num_chaves + 1; i++)
			listaNivel(raiz->desc[i], nregistros, nivel);
	}
}

void divide_no(node_Btree *x, Chave k, node_Btree **filho_direito, Chave **chave_promovida) {
	int i = x->num_chaves - 1, j;
	int chave_alocada = 0;
	node_Btree *y;

	y = cria_no();	
	y->folha = x->folha;
	y->num_chaves = (M - 1)/2;

	for(j = y->num_chaves - 1; j >= 0; j--) {
		if(!chave_alocada && strcmp(k.pk, x->chave[i].pk) > 0) {
			y->chave[j] = k;
			y->desc[j + 1] = *filho_direito;
			chave_alocada = 1;
		} else {
			y->chave[j] = x->chave[i];
			y->desc[j + 1] = x->desc[i + 1];
			i--;
		}
	}

	if(!chave_alocada) {
		while(i >= 0 && strcmp(k.pk, x->chave[i].pk) < 0) {
			x->chave[i + 1] = x->chave[i];
			x->desc[i + 2] = x->desc[i + 1];
			i--;
		}

		x->chave[i + 1] = k;
		x->desc[i + 2] = *filho_direito;
	}

	*chave_promovida = &(x->chave[M/2]);

	y->desc[0] = x->desc[(M/2)+1];
	x->num_chaves = M/2;
	*filho_direito = y;
}

void insere_aux(node_Btree *x, Chave k, node_Btree **filho_direito, Chave **chave_promovida) {
	int i;

	if(x->folha == 1) {
		if(x->num_chaves < M - 1) {
			i = x->num_chaves - 1;

			while(i >= 0 && strcmp(k.pk, x->chave[i].pk) < 0) {
				x->chave[i + 1] = x->chave[i];
				i--;
			}

			x->chave[i + 1] = k;
			x->num_chaves = x->num_chaves + 1;

			*chave_promovida = NULL;
			*filho_direito = NULL;
			return;
		} else {
			divide_no(x, k, filho_direito, chave_promovida);
			return;
		}
	} else {
		i = x->num_chaves - 1;

		while(i >= 0 && strcmp(k.pk, x->chave[i].pk) < 0) {
			i--;
		}

		i++;

		insere_aux(x->desc[i], k, filho_direito, chave_promovida);

		if(*chave_promovida != NULL) {
			k = **chave_promovida;

			if(x->num_chaves < M - 1) {
				i = x->num_chaves - 1;

				while(i >= 0 && strcmp(k.pk, x->chave[i].pk) < 0) {
					x->chave[i + 1] = x->chave[i];
					x->desc[i + 2] = x->desc[i + 1];
					i--;
				}

				x->chave[i + 1] = k;
				x->desc[i + 2] = *filho_direito;
				x->num_chaves = x->num_chaves + 1;

				*chave_promovida = NULL;
				*filho_direito = NULL;
				return;
			} else {
				divide_no(x, k, filho_direito, chave_promovida);
				return;
			}
		} else {
			*chave_promovida = NULL;
			*filho_direito = NULL;
			return;
		}
	}
}

void insere(Iprimary *iprimary, Chave k, Chave **chave_promovida, node_Btree **filho_direito) {
	node_Btree *novoNo;

	if(iprimary->raiz == NULL) {
		novoNo = cria_no();

		novoNo->folha = 1;
		novoNo->num_chaves = 1;
		novoNo->chave[0] = k;

		iprimary->raiz = novoNo;
	} else {
		insere_aux(iprimary->raiz, k, filho_direito, chave_promovida);

		if(*chave_promovida != NULL) {
			novoNo = cria_no();
			novoNo->folha = 0;
			novoNo->num_chaves = 1;
			novoNo->chave[0] = **chave_promovida;			

			novoNo->desc[0] = iprimary->raiz;
			novoNo->desc[1] = *filho_direito;

			iprimary->raiz = novoNo;
		}
	}
}

node_Btree* cria_no() {
	node_Btree *no;

	no = (node_Btree *)malloc(sizeof(node_Btree));
	no->chave = (Chave *)malloc((M - 1) * sizeof(Chave));
	no->desc = (node_Btree **)malloc(M * sizeof(node_Btree *));

	return no;
}

//Verifica se o tamanho do campo não excede 39 bytes
bool verificaTamanho(char c[]) {
	return (strlen(c) <= 39 ? TRUE : FALSE);
}

//Verifica a validade da data
bool verificaData(char data_partida[]) {
	int ano;
	int mes;
	int dia;

	ano = (((int) data_partida[6] - '0') * 1000) + (((int) data_partida[7] - '0') * 100) + (((int) data_partida[8] - '0') * 10) + (((int) data_partida[9] - '0') * 1);
	mes = (((int) data_partida[3] - '0') * 10) + (((int) data_partida[4] - '0') * 1);
	dia = (((int) data_partida[0] - '0') * 10) + (((int) data_partida[1] - '0') * 1);

	if(ano < 2011 || ano > 2015)
		return FALSE;

	if(dia < 1 || dia > 31)
		return FALSE;

	if(mes < 1 || mes > 12)
		return FALSE;

	return TRUE;
}

//Verifica se duracao da partida tem 5bytes
bool verificaDuracao(char duracao[]) {
	int min;
	int seg;

	if(strlen(duracao) != 5)
		return FALSE;

	min = (((int) duracao[0] - '0') * 10) + (((int) duracao[1] - '0') * 1);
	seg = (((int) duracao[3] - '0') * 10) + (((int) duracao[4] - '0') * 1);

	if(min < 0 || min > 99)
		return FALSE;

	if(seg < 0 || seg > 60)
		return FALSE;

	return TRUE;
}

//Verifica se abates são de 2 bytes
bool verificaAbates(char abates[]) {
	int abate;

	if(strlen(abates) != 2)
		return FALSE;

	abate = (((int) abates[0] - '0') * 10) + (((int) abates[1] - '0') * 1);

	if(abate < 0 || abate > 99)
		return FALSE;

	return TRUE;
}

//Verifica se nome da equipe vencedora é uma das duas equipes: Azul ou Vermelha
bool verificaEquipeVencedora(char equipe_vermelha[], char equipe_azul[], char vencedor[]) {

	if(!strcmp(equipe_vermelha, vencedor))
		return TRUE;

	if(!strcmp(equipe_azul, vencedor))
		return TRUE;

	return FALSE;
}

//Cria chave primária gerando a partir dos campos da partida
void criaChavePrimaria(Partida *p) {
	p->pk[0] = toupper(p->equipe_azul[0]);
	p->pk[1] = toupper(p->equipe_vermelha[0]);
	p->pk[2] = toupper(p->mvp[0]);
	p->pk[3] = toupper(p->mvp[1]);
	p->pk[4] = (p->data_partida[0]); //48 é 0 na tabela ASCII
	p->pk[5] = (p->data_partida[1]);
	p->pk[6] = (p->data_partida[3]);
	p->pk[7] = (p->data_partida[4]);
	p->pk[8] = '\0';
}

int buscaSec(char *chave, int tipo, Iwinner *winner, Imvp *mvp, int *ini, int *fim, int nItens) {
	int posicao;

	// 1 winner = tipo
	// 2 mvp 	= tipo

	if(tipo == 1) {
		posicao = buscaBinariaWinner(winner, 0, nItens, chave);

		if(posicao != -1) {
			for(*ini = posicao-1; (*ini > 0) && (strcmp(winner[posicao].vencedor, winner[*ini].vencedor) == 0); *ini = *ini - 1);
			*ini = *ini + 1;
			for(*fim = posicao+1; (*fim < nItens) && (strcmp(winner[posicao].vencedor, winner[*fim].vencedor) == 0); *fim = *fim + 1);
		}
	}
	else {
		posicao = buscaBinariaMvp(mvp, 0, nItens, chave);

		if(posicao != -1) {
			for(*ini = posicao-1; (*ini > 0) && (strcmp(mvp[posicao].mvp, mvp[*ini].mvp) == 0); *ini = *ini - 1);
			*ini = *ini + 1;
			for(*fim = posicao+1; (*fim < nItens) && (strcmp(mvp[posicao].mvp, mvp[*fim].mvp) == 0); *fim = *fim + 1);
		}
	}

	return posicao;
}

int buscaBinariaWinner(Iwinner *p, int ini, int fim, char elem[]) {
   if (fim >= ini)
   {
        int meio = ini + (fim - ini)/2;
 
        if (strcmp(p[meio].vencedor, elem) == 0)  
            return meio;
        if (strcmp(p[meio].vencedor, elem) > 0) 
            return buscaBinariaWinner(p, ini, meio-1, elem);
            
        return buscaBinariaWinner(p, meio+1, fim, elem);
   }

   return -1;
}

int buscaBinariaMvp(Imvp *p, int ini, int fim, char elem[]) {
   if (fim >= ini)
   {
        int meio = ini + (fim - ini)/2;
 
        if (strcmp(p[meio].mvp, elem) == 0)  
            return meio;
        if (strcmp(p[meio].mvp, elem) > 0) 
            return buscaBinariaMvp(p, ini, meio-1, elem);
            
        return buscaBinariaMvp(p, meio+1, fim, elem);
   }

   return -1;
}

void bubleSortWinner(Iwinner *iwinner, int nregistros) {
	int i, j;
	Iwinner aux;

	for(i = 0; i < nregistros; i++)
		for(j = i; j < nregistros; j++)
			if(cmpIndicesWinner(&iwinner[i], &iwinner[j]) > 0) {
				aux = iwinner[i];
				iwinner[i] = iwinner[j];
				iwinner[j] = aux;
			}

}

void bubleSortMvp(Imvp *mvp, int nregistros) {
	int i, j;
	Imvp aux;

	for(i = 0; i < nregistros; i++)
		for(j = i; j < nregistros; j++)
			if(cmpIndicesWinner(&mvp[i], &mvp[j]) > 0) {
				aux = mvp[i];
				mvp[i] = mvp[j];
				mvp[j] = aux;
			}
}