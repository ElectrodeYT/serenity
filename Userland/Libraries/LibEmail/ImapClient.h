#pragma once

#include <Userland/Libraries/LibEmail/EmailClient.h>
#include <Userland/Libraries/LibCore/TCPSocket.h>

namespace Email {


class ImapClient : EmailClient {
public:
    ImapClient();

    virtual bool Connect(AK::String address);
    virtual const AK::Vector<AK::String> GetMailboxes();
    virtual const AK::Vector<Email> GetMail(AK::String mailbox);

private:
    RefPtr<Core::TCPSocket> m_tcp_socket;

    AK::Vector<Email> m_fetched_mail;

    bool m_mail_fetch_complete = false;
    bool m_mail_fetch_in_progress = false;

    void HandleMessage();
    void SendMessage(AK::String message_name, AK::String message_contents);
};

}