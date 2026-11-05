# asm-reversing-lab: entry point for every task.
# Inside the lab image (ARL_IN_LAB=1) commands run directly; on the host they run in Docker.
# Use `make NATIVE=1 <target>` to run on the host with a native install.

IMAGE    ?= asm-reversing-lab
PY       ?= python3
DOCKER_RUN = docker run --rm $(if $(TTY),-it,) -v "$(CURDIR)":/lab -w /lab $(IMAGE)
RUN      = $(if $(or $(ARL_IN_LAB),$(NATIVE)),,$(DOCKER_RUN))
EX      ?=
ARGS    ?=

.PHONY: terminology help image shell build check verify repro test-snippets i18n lint all-checks clean

help:
	@echo "Targets: image shell build check verify repro test-snippets i18n lint all-checks clean"
	@echo "  make check EX=07 ARGS='--flag ARL{...}'   check one exercise"

# Build the image with the host user's uid/gid (uid 0 falls back to 1000).
HOST_UID := $(shell id -u)
HOST_GID := $(shell id -g)
LAB_UID  := $(if $(filter 0,$(HOST_UID)),1000,$(HOST_UID))
LAB_GID  := $(if $(filter 0,$(HOST_GID)),1000,$(HOST_GID))

image:
	docker build -t $(IMAGE) --build-arg LAB_UID=$(LAB_UID) --build-arg LAB_GID=$(LAB_GID) docker/

shell: image
	docker run --rm -it -v "$(CURDIR)":/lab -w /lab $(IMAGE)

build: $(if $(or $(ARL_IN_LAB),$(NATIVE)),,image)
	$(RUN) bash -c 'for m in modules/*/examples; do [ -f "$$m/Makefile" ] && $(MAKE) -C "$$m" || true; done'

check: $(if $(or $(ARL_IN_LAB),$(NATIVE)),,image)
	@test -n "$(EX)" || { echo "usage: make check EX=07 [ARGS='--flag ARL{...}']"; exit 2; }
	$(RUN) bash -c 'c=$$(ls exercises/$(EX)-*/tests/check.py 2>/dev/null | head -n1); \
	  test -n "$$c" || { echo "exercise $(EX) not found"; exit 2; }; $(PY) "$$c" $(ARGS)'

verify: $(if $(or $(ARL_IN_LAB),$(NATIVE)),,image)
	$(RUN) $(PY) tools/verify_solutions.py

repro: $(if $(or $(ARL_IN_LAB),$(NATIVE)),,image)
	$(RUN) $(PY) tools/repro_check.py

test-snippets: $(if $(or $(ARL_IN_LAB),$(NATIVE)),,image)
	$(RUN) $(PY) tools/test_snippets.py

i18n:
	$(PY) tools/i18n_check.py
	$(PY) tools/i18n_structure.py

# Advisory work list for translation reviewers: terms of i18n/terminology.csv missing from a page.
terminology:
	$(PY) tools/terminology_check.py --limit 3

lint: $(if $(or $(ARL_IN_LAB),$(NATIVE)),,image)
	$(PY) tools/no_emoji.py
	$(PY) tools/check_links.py
	$(RUN) bash -c 'markdownlint-cli2 "**/*.md" "#node_modules" "#.claude" && codespell --skip=.git,i18n,*.ar.md,*.de.md,*.es.md,*.fr.md,*.csv README.md CONTRIBUTING.md ETHICS.md glossary/glossary.md modules cheatsheets exercises solutions'

all-checks: lint build test-snippets verify repro i18n

clean:
	find modules -name build -type d -prune -exec rm -rf {} +
