#include "network/commands/base_command.hpp"

BaseCommand::~BaseCommand() = default;

std::shared_ptr<BaseCommand> BaseCommand::setNext(std::shared_ptr<BaseCommand> next) {
    next_ = next;
    return next_;
}

void BaseCommand::handle(const std::vector<std::string>& args, CommandContext context) {
    if (next_) {
        next_->handle(args, context);
    }
}
