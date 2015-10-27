/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Estruturas de Dados 2
 * Prof. Tiago A. de Almeida
 *
 * Trabalho 02 - Árvore B
 *
 * RA: 552143
 * Aluno: Diego T. A. Siqueira
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

/* Saídas do usuário */
#define OPCAO_INVALIDA "Opcao invalida!\n"
#define MEMORIA_INSUFICIENTE "Memoria insuficiente!"
#define REGISTRO_N_ENCONTRADO "Registro nao encontrado!\n\n"
#define CAMPO_INVALIDO "Campo invalido! Informe novamente.\n"
#define ERRO_PK_REPETIDA "ERRO: Ja existe um registro com a chave primaria: %s.\n"
#define ARQUIVO_VAZIO "Arquivo vazio!\n"
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
	int folha;			// flag folha da arvore 1 eh folha
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
node_Btree *filhoDireito;
Chave *chavePromovida; 
int rrnGlobal = 0;

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

Partida recuperar_registro(int rrn);
void insereArvoreB(Iprimary *iprimary, Chave *chaveInsere);
node_Btree * criarNo();
void insereArvoreBAux(node_Btree *noAtual, Chave *chaveInsere);
void divideNo(node_Btree *noAtual, Chave *chaveInsere);
Chave *buscaArvoreB(node_Btree * noBusca, char *codigo, int op);
void imprimirNo(node_Btree *noImprime);
void preordemArvoreB(node_Btree *no, int nivel);
void posordemArvoreB(node_Btree *no);

Partida inserirPartida(); //Inserir registro
void le_equipe(char []);
void le_duracao(char []);
void le_vencedora(char [], char [], char []);
void le_placar(char []);
void le_apelido_mvp(char []);
void le_data(char[]);


/* Descarta o que estiver no buffer de entrada */
void ignore() {
   char c;
   while ((c = getchar()) != '\n' && c != EOF);
}

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
		getchar();
		
		switch(opcao) {

		case 1:
			cadastrar(&iprimary, iwinner, imvp, &nregistros);
			break;
		case 2:
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
Partida recuperar_registro(int rrn){
	Partida novo;
	char *p;

	p = ARQUIVO;

	p+=rrn;

	sscanf(p, "%[^@]", novo.pk);
	p = p + strlen(novo.pk) + 1;

	sscanf(p, "%[^@]", novo.equipe_azul);
	p = p + strlen(novo.equipe_azul) + 1;

	sscanf(p, "%[^@]", novo.equipe_vermelha);
	p = p + strlen(novo.equipe_vermelha) + 1;

	sscanf(p, "%[^@]", novo.data_partida);
	p = p + strlen(novo.data_partida) + 1;

	sscanf(p, "%[^@]", novo.duracao);
	p = p + strlen(novo.duracao) + 1;

	sscanf(p, "%[^@]", novo.vencedor);
	p = p + strlen(novo.vencedor) + 1;

	sscanf(p, "%[^@]", novo.placar1);
	p = p + strlen(novo.placar1) + 1;

	sscanf(p, "%[^@]", novo.placar2);
	p = p + strlen(novo.placar2) + 1;

	sscanf(p, "%[^@]", novo.mvp);
	p = p + strlen(novo.mvp) + 1;
	
	return novo;
}
void criar_iprimary(Iprimary *iprimary, int nregistros, int ordem){
	//criar a arvore de iprimary

	int i, j;
	char *aux = ARQUIVO;
	
	Chave novo;
	Chave *busca;

	M = ordem;

	iprimary->raiz = NULL;

	for(i=0; i< nregistros; i++){
		sscanf(aux, "%[^@]", novo.pk);
		aux+= strlen(novo.pk)+1;

		busca = buscaArvoreB(iprimary->raiz, novo.pk, 0);

		if(busca != NULL){
			printf(ERRO_PK_REPETIDA, novo.pk);
		}
		else{
			novo.rrn = rrnGlobal;
			rrnGlobal+=192;
			insereArvoreB(iprimary, &novo);

			//posiciono o ponteiro aux no proximo registro
			j=0;
			while(j<8){
				if(*aux == '@')
					j++;
				aux++;
			}
			while(*aux == '#')
				aux++;

		}
	}
}
void insereArvoreB(Iprimary *iprimary, Chave *chaveInsere){

	filhoDireito = NULL;
	chavePromovida = NULL;

	node_Btree *novo;
	novo = criarNo();

	
	//nao tem raiz
	if(iprimary->raiz == NULL){
		novo->folha = 1;
		novo->num_chaves = 1;

		novo->chave[0] = *chaveInsere;
		iprimary->raiz = novo;
	}

	//caso tenha raiz
	else{
		insereArvoreBAux(iprimary->raiz, chaveInsere);

		//se deu split e ocorreu a promocao
		if(chavePromovida != NULL){
			novo->folha = 0;
			novo->num_chaves = 1;
			novo->chave[0] = *chavePromovida;

			novo->desc[0] = iprimary->raiz;
			novo->desc[1] = filhoDireito;

			iprimary->raiz = novo;
		}
	}
}
node_Btree * criarNo(){
	node_Btree *novo;
	novo = (node_Btree *)malloc(sizeof(node_Btree));
	novo->desc = (node_Btree **) malloc(M*sizeof(node_Btree *)); // *desc[m]
	novo->chave = (Chave *)malloc((M-1)*sizeof(Chave));

	return novo;
}
void insereArvoreBAux(node_Btree *noAtual, Chave *chaveInsere){
	int i;
	
	if(noAtual->folha == 1){
		if(noAtual->num_chaves < (M-1)){
			i = noAtual->num_chaves -1;

			while(i>=0 && (strcmp(chaveInsere->pk, noAtual->chave[i].pk) < 0)){
				noAtual->chave[i+1] = noAtual->chave[i];
				i--;
			}
			noAtual->chave[i+1] = *chaveInsere;
			noAtual->num_chaves++;
			chavePromovida = NULL;
			filhoDireito = NULL;
			return;
		}
		else{
			//filhoDireito = NULL;
			divideNo(noAtual, chaveInsere);
			return;
		}
	}
	else{
		i = noAtual->num_chaves-1;
		
		while(i>=0 && strcmp(chaveInsere->pk, noAtual->chave[i].pk) < 0){
			i--;
		}
		i++;

		insereArvoreBAux(noAtual->desc[i], chaveInsere);

		if(chavePromovida != NULL){
			*chaveInsere = *chavePromovida;

			if(noAtual->num_chaves < (M-1)){
				i=noAtual->num_chaves-1;

				while(i>=0 && strcmp(chaveInsere->pk, noAtual->chave[i].pk) < 0){
					noAtual->chave[i+1] = noAtual->chave[i];
					noAtual->desc[i+2] = noAtual->desc[i+1];
					i--;
				}
				noAtual->chave[i+1] = *chaveInsere;
				noAtual->desc[i+2] = filhoDireito;
				noAtual->num_chaves++;
				chavePromovida = NULL;
				filhoDireito = NULL;
			}
			else{
				divideNo(noAtual,chaveInsere);
			}
		}
		else{
			chavePromovida = NULL;
			filhoDireito = NULL;
		}
	}
}
void divideNo(node_Btree *noAtual, Chave *chaveInsere){

	int i, j, chaveAlocada=0;
	node_Btree *novo;

	i = noAtual->num_chaves-1;

	novo = criarNo();
	novo->folha = noAtual->folha;
	novo->num_chaves = ((M-1)/2); //piso de (M-1)/2


	for(j= novo->num_chaves-1; j >= 0; j--){
		if(!chaveAlocada && strcmp(chaveInsere->pk, noAtual->chave[i].pk) > 0){
			novo->chave[j] = *chaveInsere;
			novo->desc[j+1] = filhoDireito;
			chaveAlocada = 1;
		}
		else{
			novo->chave[j] = noAtual->chave[i];
			novo->desc[j+1] = noAtual->desc[i+1];
			i--;
		}
	}
	if(!chaveAlocada){

		while(i>=0 && strcmp(chaveInsere->pk, noAtual->chave[i].pk) < 0){
			noAtual->chave[i+1] = noAtual->chave[i];
			noAtual->desc[i+2] = noAtual->desc[i+1];
			i--;
		}
		noAtual->chave[i+1] = *chaveInsere;
		noAtual->desc[i+2] = filhoDireito;
	}

		chavePromovida = (Chave *) malloc(sizeof(Chave));

	*chavePromovida = noAtual->chave[(M/2)];
	novo->desc[0] =  noAtual->desc[(M/2)+1];
	noAtual->num_chaves = M/2; 

	filhoDireito = novo;
}
void criar_iwinner(Iwinner *iwinner, int nregistros){
	// colocar no vetor rrn e nome vencedor
	int i, j;
	Iwinner troca;
	char *aux = ARQUIVO;

	for(i=0; i< nregistros; i++){
		
		//le o codigo
		j=0;
		while(*aux != '@'){
			iwinner[i].pk[j] = *aux;
			aux++;
			j++;
		}
		iwinner[i].pk[j] = '\0';

		//passa os campos desnecessarios
		j=0;
		while(j<5){
			if(*aux == '@')
				j++;
			aux++;
		}

		//le o nome da equipe vencedora
		j=0;
		while(*aux != '@'){
			iwinner[i].vencedor[j] = *aux;
			aux++;
			j++;
		}
		iwinner[i].vencedor[j] = '\0';

		//passa os campos desnecessarios e coloca o ponteiro no inicio do proximo registro
		j=0;
		while(j<4){
			if(*aux == '@')
				j++;
			aux++;
		}
		while(*aux == '#')
			aux++;

	}

	//ordenar o vetor
	i=0;
	j=0;

	for(i=0; i<nregistros; i++){
		for(j=i+1; j<nregistros; j++){
			if(strcmp(iwinner[i].vencedor, iwinner[j].vencedor) > 0){
				troca = iwinner[j];
				iwinner[j] = iwinner[i];
				iwinner[i] = troca;
			}
			if(strcmp(iwinner[i].vencedor, iwinner[j].vencedor) == 0) {
				if(strcmp(iwinner[i].pk, iwinner[j].pk) > 0){
					troca = iwinner[j];
					iwinner[j] = iwinner[i];
					iwinner[i] = troca;
				}
			}
		}
	}
}
void criar_imvp(Imvp *imvp, int nregistros){
	// colocar no vetor rrn e apelido mvp

	int i, j;
	Imvp troca;
	Partida novo;

	char *aux = ARQUIVO;

	for(i=0; i< nregistros; i++){		
		
		sscanf(aux, "%[^@]", novo.pk);
		aux+= strlen(novo.pk)+1;
	//	printf("%s\n", novo.pk);

		sscanf(aux, "%[^@]", novo.equipe_azul);
		aux+= strlen(novo.equipe_azul)+1;
		//printf("%s\n", novo.equipe_azul);

		sscanf(aux, "%[^@]", novo.equipe_vermelha);
		aux+= strlen(novo.equipe_vermelha)+1;
		//printf("%s\n", novo.equipe_vermelha);

		sscanf(aux, "%[^@]", novo.data_partida);
		aux+= strlen(novo.data_partida)+1;
		//printf("%s\n", novo.data_partida);

		sscanf(aux, "%[^@]", novo.duracao);
		aux+= strlen(novo.duracao)+1;
		//printf("%s\n", novo.duracao);

		sscanf(aux, "%[^@]", novo.vencedor);
		aux+= strlen(novo.vencedor)+1;
		//printf("%s\n", novo.vencedor);

		sscanf(aux, "%[^@]", novo.placar1);
		aux+= strlen(novo.placar1)+1;
		//printf("%s\n", novo.placar1);

		sscanf(aux, "%[^@]", novo.placar2);
		aux+= strlen(novo.placar2)+1;
		//printf("%s\n", novo.placar2);

		sscanf(aux, "%[^@]", novo.mvp);
		aux+= strlen(novo.mvp)+1;
	//	printf("%s\n", novo.mvp);		
		
		while(*aux == '#')
			aux++;

		strcpy(imvp[i].mvp, novo.mvp);
		strcpy(imvp[i].pk, novo.pk);
	}

	//ordenar o vetor
	i=0;
	j=0;


	for(i=0; i<nregistros; i++){
		for(j=i+1; j<nregistros; j++){
			if(strcmp(imvp[i].mvp, imvp[j].mvp) > 0){
				troca = imvp[j];
				imvp[j] = imvp[i];
				imvp[i] = troca;
			}
			if(strcmp(imvp[i].mvp, imvp[j].mvp) == 0) {
				if(strcmp(imvp[i].pk, imvp[j].pk) > 0){
					troca = imvp[j];
					imvp[j] = imvp[i];
					imvp[i] = troca;
				}
			}
		}
	}
}
void cadastrar(Iprimary *iprimary, Iwinner *iwinner, Imvp *imvp, int *nregistros){
	char *novo;
	Partida p;

	Chave nova;
	Chave *busca;

	Iwinner trocaIwinner;
	Imvp trocaImvp;

	int t=0, i, j;

	novo = ARQUIVO;
	novo += *nregistros * 192;

	p = inserirPartida();

	busca = buscaArvoreB(iprimary->raiz, p.pk, 0);

	if(busca != NULL){
		printf(ERRO_PK_REPETIDA, p.pk);
		return;
	}
	else{
		sprintf(novo, "%s@",p.pk);
		novo+= strlen(p.pk)+1;
		t+=strlen(p.pk)+1;

		sprintf(novo,"%s@",p.equipe_azul);
		novo+= strlen(p.equipe_azul)+1;
		t+= strlen(p.equipe_azul)+1;

		sprintf(novo,"%s@",p.equipe_vermelha);
		novo+= strlen(p.equipe_vermelha)+1;
		t+= strlen(p.equipe_vermelha)+1;

		sprintf(novo,"%s@",p.data_partida);
		novo+= strlen(p.data_partida)+1;
		t+= strlen(p.data_partida)+1;

		sprintf(novo,"%s@",p.duracao);
		novo+= strlen(p.duracao)+1;
		t+= strlen(p.duracao)+1;

		sprintf(novo,"%s@",p.vencedor);
		novo+= strlen(p.vencedor)+1;
		t+= strlen(p.vencedor)+1;

		sprintf(novo,"%s@",p.placar1);
		novo+= strlen(p.placar1)+1;
		t+= strlen(p.placar1)+1;

		sprintf(novo,"%s@",p.placar2);
		novo+= strlen(p.placar2)+1;
		t+= strlen(p.placar2)+1;

		sprintf(novo,"%s@",p.mvp);
		novo+= strlen(p.mvp)+1;
		t+= strlen(p.mvp)+1;

		while(t<192){
			sprintf(novo, "#");
			novo++;
			t++;
		}

		strcpy(nova.pk, p.pk);

		nova.rrn = rrnGlobal;

		strcpy(trocaIwinner.vencedor, p.vencedor);
		strcpy(trocaIwinner.pk, p.pk);

		strcpy(trocaImvp.mvp, p.mvp);
		strcpy(trocaImvp.pk, p.pk);

		insereArvoreB(iprimary, &nova);



		iwinner[*nregistros] = trocaIwinner;
		imvp[*nregistros] = trocaImvp;


		rrnGlobal+=192;
		*nregistros+=1;

		for(i=0; i<*nregistros; i++){
			for(j=i+1; j<*nregistros; j++){
				if(strcmp(iwinner[i].vencedor, iwinner[j].vencedor) > 0){
					trocaIwinner = iwinner[j];
					iwinner[j] = iwinner[i];
					iwinner[i] = trocaIwinner;
				}
				if(strcmp(iwinner[i].vencedor, iwinner[j].vencedor) == 0) {
					if(strcmp(iwinner[i].pk, iwinner[j].pk) > 0){
						trocaIwinner = iwinner[j];
						iwinner[j] = iwinner[i];
						iwinner[i] = trocaIwinner;
					}
				}
			}
		}
		
		for(i=0; i<*nregistros; i++){
			for(j=i+1; j<*nregistros; j++){
				if(strcmp(imvp[i].mvp, imvp[j].mvp) > 0){
					trocaImvp = imvp[j];
					imvp[j] = imvp[i];
					imvp[i] = trocaImvp;
				}
				if(strcmp(imvp[i].mvp, imvp[j].mvp) == 0) {
					if(strcmp(imvp[i].pk, imvp[j].pk) > 0){
						trocaImvp = imvp[j];
						imvp[j] = imvp[i];
						imvp[i] = trocaImvp;
					}
				}
			}
		}
	}
}
void alterar(Iprimary iprimary){

	char buscacod[TAM_PRIMARY_KEY];
	char novaDuracao[5];
	Chave *aux;
	int j;

	char *p = ARQUIVO;
	
	scanf("%8[^\n]", buscacod);
	getchar();

	aux = buscaArvoreB(iprimary.raiz, buscacod, 0);

	if(aux != NULL){
		p+=aux->rrn;

		le_duracao(novaDuracao);

		j=0;
		while(j<4){
			if(*p == '@')
				j++;
			p++;
		}

		sprintf(p, "%c", novaDuracao[0]);
		p++;
		sprintf(p, "%c", novaDuracao[1]);
		p++;
		sprintf(p, "%c", novaDuracao[2]);
		p++;
		sprintf(p, "%c", novaDuracao[3]);
		p++;
		sprintf(p, "%c", novaDuracao[4]);
		p++;
	}
	else{
		printf(REGISTRO_N_ENCONTRADO);
	}
	//scanf da chave que o usuario vai procurar
	//se nao tiver mostra mensagem
	//se tiver scanf da nova duracao le_duracao
	//altera no arquivo de dados
}
Chave *buscaArvoreB(node_Btree * noBusca, char *codigo, int op){
	int i=0;

	if(noBusca == NULL){
		return NULL;
	}

	if(op == 1)
		imprimirNo(noBusca);
	
	while(i < noBusca->num_chaves && strcmp(codigo, noBusca->chave[i].pk) > 0)
		i++;

	if(i < noBusca->num_chaves && strcmp(codigo, noBusca->chave[i].pk) == 0)
		return &noBusca->chave[i];

	if(noBusca->folha == 1)
		return NULL;
	else{
		return buscaArvoreB(noBusca->desc[i], codigo, op);
	}
}
void imprimirNo(node_Btree *noImprime){
	int i=0;

	if(noImprime != NULL){
		while(i < noImprime->num_chaves){
			if(i==0)
				printf("%s", noImprime->chave[i].pk);
			if(i!=0)
				printf(", %s", noImprime->chave[i].pk);
			i++;
		}
		printf("\n");
	}
}
void buscar(Iprimary iprimary, Iwinner *iwinner, Imvp *imvp, int nregistros){

	int opc, i=0;	
	char buscacod[TAM_PRIMARY_KEY], buscamvp[TAM_MVP], buscavencedor[TAM_EQUIPE];

	scanf("%d", &opc);
	getchar();

	switch(opc){
			Chave *aux;
			aux = NULL;
			int t;
		case 1:
			//por codigo
			scanf("%8[^\n]", buscacod);
			buscacod[9] = '\0';

			if(nregistros == 0){
				printf(ARQUIVO_VAZIO);
			}
			else{
				printf(NOS_PERCORRIDOS, buscacod);

				aux = buscaArvoreB(iprimary.raiz, buscacod, 1);
				printf("\n");

				if(aux != NULL)
					exibir_registro(aux->rrn);
				else
					printf(REGISTRO_N_ENCONTRADO);
			}
			break;
		
		case 2:
			//por equipe vencedora
			
			scanf("%39[^\n]", buscavencedor);
		
			if(nregistros == 0){
				printf(ARQUIVO_VAZIO);

			}
			else{
				for(i=0; i<nregistros; i++){
					if(strcmp(iwinner[i].vencedor, buscavencedor) == 0){
						aux = buscaArvoreB(iprimary.raiz, iwinner[i].pk, 0);
						if(aux != NULL){
							exibir_registro(aux->rrn);
						}
					}
				}

				if(aux == NULL)
					printf(REGISTRO_N_ENCONTRADO);					
			}

			break;
		
		case 3:
			//por nome
			scanf("%39[^\n]", buscamvp);

			if(nregistros == 0){
				printf(ARQUIVO_VAZIO);
			}
			else{
				for(i=0; i<nregistros; i++){
					if(strcmp(imvp[i].mvp, buscamvp) == 0){
						aux = buscaArvoreB(iprimary.raiz, imvp[i].pk , 0);
						if(aux != NULL){
							exibir_registro(aux->rrn);
						}					
					}
				}
				if(aux == NULL)
					printf(REGISTRO_N_ENCONTRADO);
			}
			
			break;
		
		default:
			printf(CAMPO_INVALIDO);
			ignore();
			break;
	}
}
void listar(Iprimary iprimary, Iwinner *iwinner, Imvp *imvp, int nregistros){
	int i;
	int opc;
	Chave *p;

	scanf("%d", &opc);
	getchar();

	switch(opc){
		case 1:
			//preordem
			
			if(nregistros > 0){
				preordemArvoreB(iprimary.raiz, 1);
				printf("\n");
			}
			else{
				printf(ARQUIVO_VAZIO);
			}
			break;

		case 2:
			//vencedora
			if(nregistros > 0){
				for(i=0; i< nregistros; i++){
					p = buscaArvoreB(iprimary.raiz, iwinner[i].pk, 0);
					exibir_registro(p->rrn);
				}
			}
			else{	
				printf(ARQUIVO_VAZIO);
			}
			break;

		case 3:
			//mvp
			if(nregistros > 0){
				for(i=0; i< nregistros; i++){
					p = buscaArvoreB(iprimary.raiz, imvp[i].pk, 0);
					exibir_registro(p->rrn);
				}
			}
			else{
				printf(ARQUIVO_VAZIO);
			}
		
			break;

		default:
			printf(CAMPO_INVALIDO);
			break;
	}
}
void preordemArvoreB(node_Btree *no, int nivel){
	int i=0;

	printf("%d - ", nivel);
	
	for(i=0; i< no->num_chaves; i++){
		if(i==0)
			printf("%s", no->chave[i].pk);
		if(i!= 0)
			printf(", %s", no->chave[i].pk);
	}
	nivel++;

	printf("\n");

	if(no->folha == 0){
		for(i=0; i <= no->num_chaves; i++){
			preordemArvoreB(no->desc[i], nivel);
		}
	}
	else{
		return;
	}
}
void apagar_no(node_Btree **raiz){
	//printf("ACESO\n");
	//posordemArvoreB(*raiz);
	//printf("APAGANDO\n");	
}
void posordemArvoreB(node_Btree *no){
	
	int i=no->num_chaves-1;

	while(i > 0){
		posordemArvoreB(no->desc[i]);
		i--;
	}
	free(no);
}
Partida inserirPartida(){	
	Partida novo;

	le_equipe(novo.equipe_azul);
	le_equipe(novo.equipe_vermelha);
	le_data(novo.data_partida);
	le_duracao(novo.duracao);
	le_vencedora(novo.vencedor, novo.equipe_azul, novo.equipe_vermelha);
	le_placar(novo.placar1);
	le_placar(novo.placar2);
	le_apelido_mvp(novo.mvp);

	novo.pk[0] = toupper(novo.equipe_azul[0]);
	novo.pk[1] = toupper(novo.equipe_vermelha[0]);
	novo.pk[2] = toupper(novo.mvp[0]);
	novo.pk[3] = toupper(novo.mvp[1]);
	novo.pk[4] = novo.data_partida[0];
	novo.pk[5] = novo.data_partida[1];
	novo.pk[6] = novo.data_partida[3];
	novo.pk[7] = novo.data_partida[4];
	novo.pk[8] = '\0';

	return novo;
}

/* Leituras usadas no Trabalho 1, com algumas correções de erros. Feita em conjunto com Gabriel Alves*/
void le_equipe(char equipe_chamada[]) {
        char equipe[1000];
        int flag=0;
 
        do {
                flag = 0;
                scanf("%[^\n]", equipe);
                getchar();
                if(strlen(equipe) > 39) {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
        } while(flag);
        strcpy(equipe_chamada, equipe);
}
void le_data(char data_chamada[]) {
        char data[11];
        int flag=0;
 
        do {
                flag = 0;
                scanf("%10[^\n]", data);
                getchar();
                if(data[2] != '/' || data[5] != '/') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[0] == '0' && data[1] == '0') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;      
                }
                else if(data[3] == '0' && data[4] == '0') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[6] != '2' || data[7] != '0' || data[8] != '1') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[9] != '1' && data[9] != '2' && data[9] != '3' && data[9] != '4' && data[9] != '5') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[0] != '0' && data[0] != '1' && data[0] != '2' && data[0] != '3') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[0] == '3' && data[1] != '0' && data[1] != '1') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[3] != '0' && data[3] != '1') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(data[3] == '1' && data[4] != '0' && data[4] != '1' && data[4] != '2') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
        } while(flag);
 
        strcpy(data_chamada, data);
}
void le_duracao(char duracao_chamada[]) {
        char duracao[6];
        int flag=0;
 
        do {
                flag = 0;
                scanf("%5[^\n]", duracao);
                ignore();
 
                if(duracao[2] != ':') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(strlen(duracao) != 5) {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[0] == '0' && duracao[1] == '0' && duracao[3] == '0' && duracao[4] == '0') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[0] != '0' && duracao[0] != '1' && duracao[0] != '2' && duracao[0] != '3' && duracao[0] != '4' && duracao[0] != '5' && duracao[0] != '6' && duracao[0] != '7' && duracao[0] != '8' && duracao[0] != '9') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[1] != '0' && duracao[1] != '1' && duracao[1] != '2' && duracao[1] != '3' && duracao[1] != '4' && duracao[1] != '5' && duracao[1] != '6' && duracao[1] != '7' && duracao[1] != '8' && duracao[1] != '9') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[3] != '0' && duracao[3] != '1' && duracao[3] != '2' && duracao[3] != '3' && duracao[3] != '4' && duracao[3] != '5' && duracao[3] != '6' && duracao[3] != '7' && duracao[3] != '8' && duracao[3] != '9') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[3] == '7' || duracao[3] == '8' || duracao[3] == '9'){
                		printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[3] == '6' && duracao[4] != '0'){
                		printf(CAMPO_INVALIDO);
                        flag = 1;
                }
                else if(duracao[4] != '0' && duracao[4] != '1' && duracao[4] != '2' && duracao[4] != '3' && duracao[4] != '4' && duracao[4] != '5' && duracao[4] != '6' && duracao[4] != '7' && duracao[4] != '8' && duracao[4] != '9') {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
 
        } while(flag);
 
        strcpy(duracao_chamada, duracao);      
}
void le_vencedora(char vencedora_chamada[], char equipe_azul[], char equipe_vermelha[]) {
        int flag;
        char vencedora[40];
 
        do{
                flag = 0;
                scanf("%39[^\n]", vencedora);
                getchar();
 
                if(strcmp(equipe_azul, vencedora) != 0 && strcmp(equipe_vermelha, vencedora) != 0) {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
        }while(flag);
 
        strcpy(vencedora_chamada, vencedora);
}
void le_placar(char placar_chamada[]) {
        char placar[3];
        int tamanho_placar, flag=1;
 
        do {
                tamanho_placar = 0;
                flag = 0;
                scanf("%s[^\n]", placar);
                getchar();
                tamanho_placar = strlen(placar);
                if(tamanho_placar != 2 || (placar[0] != '0' && placar[0] != '1' && placar[0] != '2' && placar[0] != '3' && placar[0] != '4' && placar[0] != '5' && placar[0] != '6' && placar[0] != '7' && placar[0] != '8' && placar[0] != '9') || (placar[1] != '0' && placar[1] != '1' && placar[1] != '2' && placar[1] != '3' && placar[1] != '4' && placar[1] != '5' && placar[1] != '6' && placar[1] != '7' && placar[1] != '8' && placar[1] != '9') ) {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
        } while (flag);
 
        strcpy(placar_chamada, placar);
}
void le_apelido_mvp(char apelido_chamada[]) {
        char apelido_mvp[1000];
        int flag=0;
 
        do {
                flag = 0;
                scanf("%[^\n]", apelido_mvp);
                getchar();
                if(strlen(apelido_mvp) > 39) {
                        printf(CAMPO_INVALIDO);
                        flag = 1;
                }
        } while(flag);
 
        strcpy(apelido_chamada, apelido_mvp);
}
