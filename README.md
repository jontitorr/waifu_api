# Waifu API

A small HTTP server application that serves data about your favorite waifus.

## Why?

This project was made to learn more about CI/CD with Terraform and AWS, as well as to make use of the MongoDB Atlas Data API. This project is **not** a production-ready solution for your REST API needs, but a way to get acquainted with the mentioned technologies. If you wish to use this project you will need to set up your own MongoDB Atlas cluster with the Data API enabled.

## Features

## Getting Started

### Prerequisites

- [AWS](https://aws.amazon.com/) (required for infrastructure deployment)
- [CMake](https://cmake.org/) (version >= 3.15)
- Compiler with C++17 support, i.e. MSVC, GCC, Clang
- [MongoDB Atlas](https://www.mongodb.com/cloud/atlas) (You will need to create a cluster and enable the [Data API](https://www.mongodb.com/developer/products/atlas/atlas-data-api-introduction/) for it.)
- [Terraform](https://www.terraform.io/) (required for infrastructure deployment)

### Installing

```bash
  git clone https://www.github.com/xminent/waifu_api
  cd waifu_api/api
  cmake -S . -B build
  cmake --build build --target install
```

In order to run the server you need to supply the following environment variables:

- `MONGO_DB_API_KEY` - The API key for the MongoDB Atlas cluster
- `MONGO_DB_CLUSTER_NAME` - The name of the MongoDB Atlas cluster
- `MONGO_DB_COLLECTION_NAME` - The name of the collection in the MongoDB Atlas cluster which contains the waifu data
- `MONGO_DB_DATABASE_NAME` - The name of the database in the MongoDB Atlas cluster which contains the waifu data collection
- `MONGO_DB_URL` - The URL to the MongoDB Atlas cluster

### Deploying

If you want to build the infrastructure for this project, you can do so by running the following commands:

```bash
  cd waifu_api/terraform
  terraform init
  terraform apply
```

This will provision the following resources:

- An EC2 instance running Ubuntu 22.04 LTS
- A security group for the EC2 instance which allows both HTTP and SSH traffic
- An SSH key pair for the EC2 instance so you can connect to it

> You will need to have your AWS credentials set up for this to work and will have to pass the prompted variables, such as the AWS bucket name, private SSH key, etc.

After `terraform apply`ing, you can then read the output variable `server_public_ip` to get the public IP of the server and connect to it using HTTP. Setting up HTTPS is left as an exercise for the reader. If you wish to see an application of this API, see my [personal website](https://xminent.com).

## Usage

### Routes

You can configure as many routes as you want, just edit the `src/main.cpp` file. The following routes are configured by default:

- `/waifu/{name}`
  - Returns a waifu with the given name. This route expects your data to contain a `name` field.

## Dependencies

### Third-party Dependencies

- [net](https://github.com/xminent/net) (bundled with project)

## License

[MIT](https://choosealicense.com/licenses/mit/)
