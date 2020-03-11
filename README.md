# carrinhos-e-estacoes
## Projeto de programação E Interfaceamento Hardware/Software

Nosso projeto consiste em programar um Rover que:

- Ande pelo circuito seguindo a linha
- Evite colisões frontais em caso de obstáculo
- Pare nas estações de carga e descarga por um tempo pré-programado

## Estrutura do programa

Nosso programa foi estruturado em um loop principal com 3 estágios:

### Leitura de sensores

- Leitura de sensores IR para detecção de faixa e estações
- Leitura do sensor de distancia
- Leitura do sensor RFID

### Atualização da máquina de estados interna

A fim de implementar a parada nas estações, desenvolvemos uma máquina de estados cuja saída é simplementes a velocidade que o carro deve andar.

Os estados são:

- **Andando**: O carro se move na velocidade máxima.

  Vai para o próximo estado se for detectada uma estação no sensor lateral

- **Parando na estação**: O carro freia para entrar na estação

  Vai para o próximo estado após 1s

- **Avançando na estação**: O carro se move lentamente para frente enquanto o sensor lateral estiver ativo

  Vai para o próximo estado se o sensor lateral não estiver mais ativo

- **Retroceder na estação**: O carro se move lentamente para trás até que o sensor lateral esteja ativo novamente

  Vai para o próximo estado se o sensor lateral estiver ativo novamente.

  Vai para o estado _saindo da estação_ após 3s, indicando uma situação de erro (e.g., Reposicionamento manual do carrinho)

- **Esperando na estação**: O carro está alinhado perfeitamente com a estação, fica parado pelo tempo determinado

  Vai para o _saindo da estação_ após 1s, se nenhum cartão foi lido.

  Vai para o _saindo da estação_ após 10s, se o cartão lido foi _estação de carga_.

  Vai para o _saindo da estação_ após 40s, se o cartão lido foi _estação de descarga_.

- **Saindo da estação**: O carro anda em velocidade máxima

  Vai para o _Anda_ após 1s do sensor lateral inativo.


Além disso, ao ler um cartão RFID que não seja das estações desejadas (carga e descarga), a máquina de estados pula diretamente para _Saindo da estação_, de forma a passar rapidamente.


### Seguidor de linha

Tendo como entrada os sensores e a velocidade desejada (fornecida pela máquina de estados), este estágio atua nos motores:

1. Se for detectado desalinhamento com a pista, o carro gira no lugar em velocidade máxima para corrigir a direção;
2. Se forem detectadas faixas laterais, a velocidade é limitada a 30% da velocidade máxima.
3. Se for detectado um obstáculo pelo sensor ultrassonico, a velocidade é reduzida:
    - Inalterada acima de 20cm
    - Zero abaixo de 10cm
    - Proporcional no intervalo entre 10cm e 20cm.


## Desafios

### Alinhamento com estações

Alinhar corretamente com as estações se mostrou uma tarefa surpreendemente complexa, em especial dado que:
- O carrinho anda na velocidade máxima, e requer alguns centimetros para frear.
- O carrinho o necessita de um alinhamento exato com as estações de carga e descarga.

Outro dos desafios foi garantir a leitura do cartão RFID: A leitura é algumas vezes lenta, e outras vezes é necessário avançar alguns centimetros após encontrar o sensor lateral para efetuar a leitura.

Para garantir a leitura do leitor RFID, nosso carrinho passe pela estação e depois retorna de ré, cobrindo toda a região onde o cartão pode estar localizado.
Isso também ajuda no alinhamento, uma vez que a ré é feita em velocidade reduzida e permite um alinhamento preciso.

Além disso, mesmo cobrindo toda a região onde o cartão pode estar localizado, a leitura pode demorar um pouco, e nosso carrinho aguarda alguns segundos antes de sair de uma estação "desconhecida".

### Perda de carga

Nosso carrinho leva uma pequena carga no seu "porta malas". Porém tivemos problemas com a carga caindo em diversos cenários:
- Ao aceletrar/frear, em particular ao sair das estações e das zonas de velocidade reduzida
- Nas curvas
- Na rampa da estação de pesagem

Nossa solução, foi adicionar um lixa na superficie, aumentando o atrito entre o carro e a carga, e adicionar uma peça atrás do porta-malas que impedisse a queda da carga.

### Ultrassom
A leitura do ultrassom leva vários ms, durante os quais o programa ficava travado.

Devido a isso, ao adicionar o ultrassom, o seguidor de linha tinha dificuldade em permanecer no trajeto, uma vez que os sensores a atuadores eram atualizados a uma taxa muito mais baixa, exigindo que a velocidade do carrinho fosse reduzida.

Nossa solução foi o uso de uma biblioteca diferente para acessar o ultrassom: A nova biblioteca recebe o pulso de retorno com interrupções ao invés de bloquear o programa principal.


### Estação de pesagem
Devido ao pequeno desnível, a estação de pesagem era detectada pelo sensor ultrassonico como um obstáculo, fazendo com que o carrinho parasse.

Nossa solução foi diminuir a distância de frenagem para 20cm (antes eram 35cm)


### Depuração
Uma das dificuldades principais durante o desenvolvimento é sempre visualizar o estado interno do carrinho para entender os problemas.

Infelizmente, a comunicação serial não funcionou de forma estável quando conectada aos computadores do laboratório. Além disso, a pista de testes ficava distante dos computadores.

Como alternativa, usamos um display de caracteres para exibir informações como velocidade, distancia medida pelo sensor ultrassonico, estado dos sensores de linha e de estação, e estado interno do carrinho.

Assim como o sensor ultrassonico, o uso do LCD diminui a velocidade do programa e torna o seguidor de linha mais instável. Ao invés de desenvolver um versão assíncrona, simplementes removemos a maioria dos comandos de escrita no LCD na versão final.

### Motor
Apesar de trivial, ajustar os pinos de direção e o PWM dos motores logo se tornaram uma tarefa repetitiva, e empacotamos isso tudo em uma biblitoeca `Motor`:

- `motor.setSpeed(1)` gira o motor para frente a toda velocidade
- `motor.setSpeed(-1)` gira no sentido reverso
- `motor.stop()` curto-circuita os terminais do motor, fazendo com que ele freie.
- etc
