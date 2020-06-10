//
// Created by Lastprism on 2020/6/5.
//

#ifndef MYSQL_CONNECTOR_NONCOPYABLE_H
#define MYSQL_CONNECTOR_NONCOPYABLE_H

class noncopyable{
public:
    noncopyable() = default;
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
    noncopyable(noncopyable &&) = default;
    noncopyable &operator=(noncopyable &&) = default;
    ~noncopyable() = default;
};


#endif //MYSQL_CONNECTOR_NONCOPYABLE_H
