/*
 * RMT.h
 *
 *  Created on: 20 de nov de 2017
 *      Author: Ewerton L. de Sousa
 */

#ifndef MAIN_RMT_H_
#define MAIN_RMT_H_

void INIT_RMT_TX(int CANAL, int PINO);
void RMT_SEND(rmt_item32_t* item, int NUM_BITS, int CANAL);

void imprimir_ITEM(rmt_item32_t* item, int num_de_bits);

//Teste
void RMT_TV_Ligar(rmt_item32_t* item);
void RMT_TV_CANAL_MAIS(rmt_item32_t* item);
void RMT_TV_CANAL_MENOS(rmt_item32_t* item);


#endif /* MAIN_RMT_H_ */
