#include <Userland/Libraries/LibEmail/ImapClient.h>

namespace Email {

ImapClient::ImapClient()
    : m_tcp_socket(Core::TCPSocket::construct()) {
    }

bool ImapClient::Connect(AK::String address) {
    // Make TCPSocket connect to the imap server at port 143
    m_tcp_socket->connect(address, 143);
    VERIFY(m_tcp_socket->is_open());


    return true;
}

const AK::Vector<AK::String> ImapClient::GetMailboxes() {
    VERIFY_NOT_REACHED();
}

const AK::Vector<ImapClient::Email> ImapClient::GetMail(AK::String mailbox) {
    VERIFY_NOT_REACHED();
}

}