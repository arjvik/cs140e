#include <stdio.h>
#include <stdlib.h>
#include <libssh/libssh.h>

void generate_ssh_keypair(const char *private_key_path, const char *public_key_path);

int custom_random(void *buf, size_t len, int strong) {
    unsigned char *dest = buf;
    while (len-- > 0)
        *dest++ = 0;
    return SSH_OK;
}

void generate_ssh_keypair(const char *private_key_path, const char *public_key_path) {
    ssh_set_random_callback(custom_random);
    ssh_init();

    ssh_key newkey;
    ssh_pki_generate(SSH_KEYTYPE_ED25519, 0, &newkey);

    ssh_pki_export_privkey_file(newkey, NULL, NULL, NULL, private_key_path);

    ssh_pki_export_pubkey_file(newkey, public_key_path);

    ssh_key_free(newkey);
    
    ssh_finalize();
}

int main() {
    generate_ssh_keypair("id_ed25519", "id_ed25519.pub");
    printf("Ed25519 key pair generated successfully.\n");
    return 0;
}