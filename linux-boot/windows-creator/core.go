package main

import (
	"bytes"
	"compress/gzip"
	"crypto/sha256"
	_ "embed"
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

//go:embed medium.gz
var delta []byte

type block struct {
	Extra          byte
	Offset, Length uint32
}

func assemble(src, dst string) error {
	base, e := os.Open(src)
	if e != nil {
		return e
	}
	defer base.Close()
	gz, e := gzip.NewReader(bytes.NewReader(delta))
	if e != nil {
		return e
	}
	defer gz.Close()
	var count uint32
	if e = binary.Read(gz, binary.LittleEndian, &count); e != nil {
		return e
	}
	ops := make([]block, count)
	if e = binary.Read(gz, binary.LittleEndian, ops); e != nil {
		return e
	}
	payload, e := io.ReadAll(gz)
	if e != nil {
		return e
	}
	extra := bytes.NewReader(payload)
	out, e := os.Create(dst)
	if e != nil {
		return e
	}
	defer out.Close()
	h := sha256.New()
	w := io.MultiWriter(out, h)
	var total int64
	for _, op := range ops {
		var source io.ReaderAt = base
		if op.Extra == 1 {
			source = extra
		}
		n := int64(op.Length)
		if _, e = io.CopyN(w, io.NewSectionReader(source, int64(op.Offset), n), n); e != nil {
			return e
		}
		total += n
	}
	if total != size || fmt.Sprintf("%x", h.Sum(nil)) != hash {
		return fmt.Errorf("ISO-Prüfsumme ungültig.")
	}
	return out.Sync()
}
