#include "simpleshardkv.h"

/**
 * This method is analogous to a hashmap lookup. A key is supplied in the
 * request and if its value can be found, you should set the appropriate
 * field in the response. Otherwise, you should return an error.
 *
 * @param context - you can ignore this
 * @param request a message containing a key
 * @param response we store the value for the specified key here
 * @return ::grpc::Status::OK on success, or
 * ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "<your error message
 * here>")
 */
::grpc::Status SimpleShardkvServer::Get(::grpc::ServerContext* context,
                                        const ::GetRequest* request,
                                        ::GetResponse* response) {
	const std::lock_guard<std::mutex> lock(map_mutex);
	std::string key = request->key();
	if (server_map.find(key) != server_map.end()) {
	  response->set_data(server_map[key]);
	  return ::grpc::Status::OK;
	}
	return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "key not found");
}

/**
 * Insert the given key-value mapping into our store such that future gets will
 * retrieve it.
 * If the item already exists, you must replace its previous value.
 *
 * @param context - you can ignore this
 * @param request A message containing a key-value pair
 * @param response An empty message, as we don't need to return any data
 * @return ::grpc::Status::OK on success, or
 * ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "<your error message
 * here>")
 */
::grpc::Status SimpleShardkvServer::Put(::grpc::ServerContext* context,
                                        const ::PutRequest* request,
                                        Empty* response) {
	const std::lock_guard<std::mutex> lock(map_mutex);
	server_map[request->key()] = request->data();
	return ::grpc::Status::OK;
}

/**
 * Appends the data in the request to whatever data the specified key maps to.
 * If the key is not mapped to anything, this method should be equivalent to a
 * put for the specified key and value.
 *
 * @param context - you can ignore this
 * @param request A message containing a key-value pair
 * @param response An empty message, as we don't need to return any data
 * @return ::grpc::Status::OK on success, or
 * ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "<your error message
 * here>")
 */
::grpc::Status SimpleShardkvServer::Append(::grpc::ServerContext* context,
                                           const ::AppendRequest* request,
                                           Empty* response) {
	const std::lock_guard<std::mutex> lock(map_mutex);
	if (server_map.find(request->key()) != server_map.end()) {
	  server_map[request->key()] = server_map[request->key()] + ", " + request->data();
	} else {
	  server_map[request->key()] = request->data();
	}
	return ::grpc::Status::OK;
}

/**
 * Deletes the key-value pair associated with this key from the server.
 * If this server does not contain the requested key, do nothing and return
 * the error specified
 *
 * @param context - you can ignore this
 * @param request A message containing the key to be removed
 * @param response An empty message, as we don't need to return any data
 * @return ::grpc::Status::OK on success, or
 * ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "<your error message
 * here>")
 */
::grpc::Status SimpleShardkvServer::Delete(::grpc::ServerContext* context,
                                           const ::DeleteRequest* request,
                                           Empty* response) {
	const std::lock_guard<std::mutex> lock(map_mutex);
	auto entry = server_map.find(request->key());
	if (entry != server_map.end()) {
	  server_map.erase(entry);
	  return ::grpc::Status::OK;
	}
	return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "key not exist in delete");
}
