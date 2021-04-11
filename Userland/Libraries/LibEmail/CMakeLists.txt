set(SOURCES
    EmailClient.cpp
    ImapClient.cpp
)

serenity_lib(LibEmail email)
target_link_libraries(LibEmail LibC)
