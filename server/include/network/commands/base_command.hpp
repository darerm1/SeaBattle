#pragma once

#include <memory>
#include "icommand.hpp"

class BaseCommand : public ICommand, public std::enable_shared_from_this<BaseCommand> {
public:
    virtual ~BaseCommand();

    std::shared_ptr<BaseCommand> setNext(std::shared_ptr<BaseCommand> next);

    void handle(const std::vector<std::string>& args, CommandContext context) override;

protected:
    std::shared_ptr<BaseCommand> next_;
};
