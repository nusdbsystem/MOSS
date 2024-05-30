package req

import (
	"encoding/json"
)

type InvokeRequest struct {
	ModelType       string `json:"model_type"`
	UserId          string `json:"user_id"`
	KeyServiceAddr  string `json:"key_service_address"`
	KeyServicePort  int    `json:"key_service_port"`
	EncryptedSample string `json:"encrypted_sample"`
}

func (r *InvokeRequest) Load(content []byte) (err error) {
	return json.Unmarshal(content, r)
}

type FPInvokeRequest struct {
	Name string        `json:"name"`
	Req  InvokeRequest `json:"request"`
}

func (r *FPInvokeRequest) Load(content []byte) (err error) {
	return json.Unmarshal(content, r)
}

type FPCreateRequest struct {
	Name        string `json:"name"`
	Kind        string `json:"kind"`
	Image       string `json:"image"`
	Concurrency int    `json:"concurrency"`
	Budget      int    `json:"budget"`
}

func (r *FPCreateRequest) Load(content []byte) (err error) {
	return json.Unmarshal(content, r)
}

type FPDeleteRequest struct {
	Name string `json:"name"`
}

func (r *FPDeleteRequest) Load(content []byte) (err error) {
	return json.Unmarshal(content, r)
}
