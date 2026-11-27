package main

import (
	"path/filepath"
	"testing"
)

func TestRecoveryImage(t *testing.T) {
	if e := assemble("../work/base.iso", filepath.Join(t.TempDir(), "boot.img")); e != nil {
		t.Fatal(e)
	}
}
