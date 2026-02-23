# ROADMAP - Petronilho Core V5

## Status Atual (v1.0 - Produção)
- Ingestão UDP: funcional, 48M pacotes/300s
- Arena Allocator: O(1), P50 = 2.7µs
- Ring Buffer: lock-free, 300s sem crash
- Protocol Decoder: JSON, FIX, HTTP

## Próximas Versões

### v1.1 - Indexação (geometric_wing)
- BTreeNode64: indexar pacotes por Security ID
- Node24 RB-Tree: order book por preço
- Impacto estimado no P99: a medir

### v1.2 - Matching Engine
- Integração BTree + Arena
- Persistência de estado entre reinicializações

### v1.3 - API de Consumo
- Interface para sistemas externos lerem dados
- Dashboard web mínimo